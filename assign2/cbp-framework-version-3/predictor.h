/* Author: Jared Stark;   Created: Tue Jul 27 13:39:15 PDT 2004
 * Description: This file defines a gshare branch predictor.
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class
#include "genann.h"
#include <iostream>

class PREDICTOR
{
  public:
    typedef uint32_t address_t;

  private:
    typedef uint32_t history_t;
    typedef uint8_t counter_t;

    static const int BHR_LENGTH = 15;
    static const int BHR_LENGTH_NN = 24;
    // never changes. Used to add 1 in the bhr by O Ring.
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const history_t BHR_MSB_NN = (history_t(1) << (BHR_LENGTH_NN - 1));
    static const std::size_t PHT_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT_INDEX_MASK = (PHT_SIZE - 1);
    static const std::size_t ADDRESS_SIZE = 16;
    static const std::size_t ADDRESS_MASK = (ADDRESS_SIZE != 32) ? ((std::size_t(1) << ADDRESS_SIZE) - 1):(-1);
    static const counter_t PHT_INIT = /* weakly taken */ 2;
    static const std::size_t INPUT_LENGTH = BHR_LENGTH_NN + ADDRESS_SIZE + 1 + 1;
    double training_data_input[INPUT_LENGTH];
    genann *ann;
    bool prediction = false;
    int predicGshare = 0;
    uint32_t TIMES = 2;
    uint32_t count = 0;

    history_t bhr;                // 15 bits
    history_t bhr_nn;                // 15 bits
    std::vector<counter_t> pht;   // 64K bits

    void update_bhr(bool taken) { 
        bhr >>= 1; if (taken) bhr |= BHR_MSB;
        bhr_nn >>= 1; if(taken) bhr_nn |= BHR_MSB_NN;
    }
    static std::size_t pht_index(address_t pc, history_t bhr) 
        { return (static_cast<std::size_t>(pc ^ bhr) & PHT_INDEX_MASK); }
    //Assuming 2 bit counter
    static bool counter_msb(/* 2-bit counter */ counter_t cnt) { return (cnt >= 2); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
        { if (cnt != 3) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
        { if (cnt != 0) --cnt; return cnt; }
    void fill_input(address_t pc){
        uint32_t temp = bhr_nn;
        for (int i = 0; i < BHR_LENGTH_NN; i++) {
            training_data_input[i] = double(temp & 1);
            temp >>= 1;
        }
        address_t masked_address = pc & ADDRESS_MASK;
        for (int i = 0; i < ADDRESS_SIZE; i++) {
            training_data_input[i+BHR_LENGTH_NN] = double(masked_address & 1);
            masked_address >>= 1;
        }
    }

  public:
    PREDICTOR(void) : bhr(0), bhr_nn(0), pht(PHT_SIZE, counter_t(PHT_INIT)) {
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
            prediction = false;
            predicGshare = 0;
            if (/* conditional branch */ br->is_conditional) {
                address_t pc = br->instruction_addr;
                std::size_t index = pht_index(pc, bhr);
                counter_t cnt = pht[index];
                predicGshare = counter_msb(cnt);
                training_data_input[INPUT_LENGTH-2] = double(predicGshare*20);
                training_data_input[INPUT_LENGTH-1] = double(1);
                fill_input(pc);
                //std::cout << *genann_run(ann, training_data_input) << std::endl;
                prediction = (*genann_run(ann, training_data_input) >= 0.5)? 1:0; 
                count += prediction ^ predicGshare;
                //std::cout<<prediction<<std::endl;
            }
            return prediction;   // true for taken, false for not taken
        }

    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {
            if (/* conditional branch */ br->is_conditional) {
                address_t pc = br->instruction_addr;
                std::size_t index = pht_index(pc, bhr);
                counter_t cnt = pht[index];
                if (taken)
                    cnt = counter_inc(cnt);
                else
                    cnt = counter_dec(cnt);
                pht[index] = cnt;
                fill_input(pc);
                training_data_input[INPUT_LENGTH-2] = double(predicGshare*20);
                training_data_input[INPUT_LENGTH-1] = double(1);
                for (int i = 0; i < TIMES; i++) {
                    double output[1];
                    output[0] = taken;
                    genann_train(ann, training_data_input, output, 0.00005);
                }
                update_bhr(taken);
                /*for (int i = 0; i < INPUT_LENGTH; i++) {
                    std::cout<<training_data_input[i] << ' ';
                }
                for (int i = 0; i < INPUT_LENGTH; i++) {
                    std::cout << ann->weight[i] << ' ';
                }
                std::cout <<std::endl;*/
            }
        }
    void print(){
        for (int i = 0; i < INPUT_LENGTH*2+5; i++) {
            std::cout << ann->weight[i] << ' ';
        }
        std::cout <<std::endl<<count << std::endl;
        
    }
};

#endif // PREDICTOR_H_SEEN

