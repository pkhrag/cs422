/* Author: Jared Stark;   Created: Tue Jul 27 13:39:15 PDT 2004
 * Description: This file defines a gshare branch predictor.
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <iostream>
#include <stdlib.h>
#include <cstddef>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class
#include "genann.h"

class PREDICTOR
{
  public:
    typedef uint32_t address_t;

  private:
    typedef uint32_t history_t;
    typedef int8_t counter_t;
    typedef unsigned __int128 longhistory_t;

    static const int BHR_LENGTH = 64;  // 64 bits
    static const int BHR_LENGTH_NN = 64;  // 64 bits
    static const uint64_t BHR_MASK = (BHR_LENGTH_NN!=64)?((uint64_t(1) << BHR_LENGTH_NN) - 1):(-1);  // 64 bits
    static const int PHIST_LENGTH = 16; // 16 bits
    static const int PHT_LENGTH = 11;
    static const int PHT_COUNT = 8;    // 8 tables
    static const longhistory_t BHR_MSB = (longhistory_t(1) << (BHR_LENGTH - 1));
    static const history_t PHIST_MSB = (history_t(1) << (PHIST_LENGTH - 1));
    static const std::size_t PHT_SIZE = (std::size_t(1) << PHT_LENGTH);
    static const counter_t PHT_INIT = 4;
    static const counter_t TC_BITS = 5;  // 5 bits
    static const std::size_t INPUT_LENGTH = BHR_LENGTH_NN + PHIST_LENGTH + PHT_COUNT + 1 + 1;
    static const uint32_t TIMES = 2;
    double training_data_input[INPUT_LENGTH];

    genann *ann;
    bool predict_nn;
    longhistory_t bhr;
    history_t phist;
    counter_t tc;
    uint32_t theta;
    std::vector<counter_t> phts[PHT_COUNT];   // 64K bits

    static address_t revBits(address_t n) {
      uint32_t bitCount = sizeof(address_t)*8;
      uint32_t reversed = 0, temp;
      for(uint32_t i=0; i<bitCount; ++i) {
        temp = n & (1 << i);
        if (temp)
          reversed |= (1 << ((bitCount-1)-i));
      }
      return reversed;
    }
    void update_bhr(bool taken) { bhr <<= 1; if (taken) bhr |= 1; }
    void update_phist(address_t pc) { phist <<= 1; phist |= pc & 1; }
    static std::size_t pht_index(address_t pc, longhistory_t bhr, history_t phist, int pht) {
      std::size_t PHT_MASK = (size_t(1) << PHT_LENGTH)-1;
      if(pht == 0) {
        return static_cast<std::size_t>(pc & PHT_MASK);
      }
      uint32_t bhrBitCount = (pht==0 ? 0 : (uint32_t(1) << (pht-1)));
      uint32_t phistBitCount = std::min(uint32_t(16), bhrBitCount);
      uint64_t append = phist & ((1 << phistBitCount) - 1);
      append |= ((bhr & ((1 << bhrBitCount) - 1))  << phistBitCount);
      append |= (longhistory_t(pc) << (bhrBitCount + phistBitCount));

      std::size_t ret = append & PHT_MASK;
      append >>= PHT_LENGTH;
      ret ^= (append & PHT_MASK);
      append >>= PHT_LENGTH;
      ret ^= (append & PHT_MASK);
      return ret;
    }
    static bool counter_msb(counter_t cnt) { return (cnt >= 0); }
    static counter_t counter_inc(counter_t cnt)
        { if (cnt != 7) ++cnt; return cnt; }
    static counter_t counter_dec(counter_t cnt)
        { if (cnt != -8) --cnt; return cnt; }
    void tc_inc(){
      if (tc != ((counter_t(1) << (TC_BITS-1)) - 1))
        ++tc;
      else {
        tc= 0;
        ++theta;
      }
      return;
    }
    void tc_dec(){
      if (tc != (counter_t(1) << (TC_BITS-1)))
        --tc;
      else {
        tc= 0;
        --theta;
      }
      return;
    }
    void fill_input(address_t pc){
        uint64_t masked_bhr = bhr & BHR_MASK;
        for (int i = 0; i < BHR_LENGTH_NN; ++i)
        {
            training_data_input[i] = double(masked_bhr & 1);
            masked_bhr >>= 1;
        }
        uint32_t temp_phist = phist;
        for (int i = 0; i < PHIST_LENGTH; ++i)
        {
            training_data_input[BHR_LENGTH_NN + i] = double(temp_phist & 1);
            temp_phist >>= 1;
        }
        for (int i = 0; i < PHT_COUNT; ++i)
        {
            std::size_t index = pht_index(pc, bhr, phist, i);
            counter_t cnt = phts[i][index];//bias
            training_data_input[i + BHR_LENGTH_NN + PHIST_LENGTH] = 2*(double(cnt) + 0.5);
        }
        training_data_input[INPUT_LENGTH-1]= double(1);
    }

  public:
 PREDICTOR(void) : bhr(0), phist(0), theta(PHT_COUNT/2) {
      std::vector<counter_t> init(PHT_SIZE, 4);
      for(int i=0; i<PHT_COUNT; ++i) {
        phts[i] = init;
      }
      ann = genann_init(INPUT_LENGTH, 1, 7, 1);
    }
    // uses compiler generated copy constructor
    // uses compiler generated destructor
    // uses compiler generated assignment operator

    // get_prediction() takes a branch record (br, branch_record_c is defined in
    // tread.h) and architectural state (os, op_state_c is defined op_state.h).
    // Your predictor should use this information to figure out what prediction it
    // wants to make.  Keep in mind you're only obligated to make predictions for
    // conditional branches.
    bool get_prediction(const branch_record_c* br, const op_state_c* os)
        {
            bool prediction = false;
            if (/* conditional branch */ br->is_conditional) {
              float prediction_value = get_prediction_value(br, os);
              address_t pc = br->instruction_addr;
              prediction = (prediction_value >= 0 ? true : false);
              training_data_input[INPUT_LENGTH-2] = (double(prediction) - 0.5)*10;
              fill_input(pc);
              predict_nn = (*genann_run(ann, training_data_input) >= 0.5)? 1:0;
              // std::cout<< *genann_run(ann, training_data_input) << std::endl;
            }
            // return prediction;   // true for taken, false for not taken
            return predict_nn;
        }

    float get_prediction_value(const branch_record_c* br, const op_state_c* os) {
      address_t pc = br->instruction_addr;
      float prediction_value = PHT_COUNT/2.0;
      for(int i=0; i<PHT_COUNT; ++i){
        std::size_t index = pht_index(pc, bhr, phist, i);
        counter_t cnt = phts[i][index];
        prediction_value += cnt;
      }
      return prediction_value;
    }
    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {
                address_t pc = br->instruction_addr;
          if (/* conditional branch */ br->is_conditional) {
                bool prediction = get_prediction(br, os);
                float prediction_value = get_prediction_value(br, os);
                if(prediction != taken || abs(prediction_value) < theta) {
                  for(int i=0; i<PHT_COUNT; ++i){
                    std::size_t index = pht_index(pc, bhr, phist, i);
                    counter_t cnt = phts[i][index];
                    if (taken)
                      cnt = counter_inc(cnt);
                    else
                      cnt = counter_dec(cnt);
                    phts[i][index] = cnt;
                  }
                }
                if(prediction != taken) {
                  tc_inc();
                } else if (abs(prediction_value) <= theta) {
                  tc_dec();
                }
                training_data_input[INPUT_LENGTH-2] = (double(prediction) - 0.5)*10;
                for (int i = 0; i < TIMES; i++) {
                    double output[1];
                    output[0] = taken;
                    genann_train(ann, training_data_input, output, 0.0005);
                }
                update_bhr(taken);
                update_phist(pc);
          } else if (br->is_return || br->is_call) {
            update_bhr(true);
            update_phist(pc);
          }
        }
};

#endif // PREDICTOR_H_SEEN