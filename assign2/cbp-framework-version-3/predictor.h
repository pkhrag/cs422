/* Author: Jared Stark;   Created: Tue Jul 27 13:39:15 PDT 2004
 * Description: This file defines a gshare branch predictor.
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <cstddef>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class

class PREDICTOR
{
  public:
    typedef uint32_t address_t;

  private:
    typedef uint64_t history_t;
    typedef uint8_t counter_t;

    // static const int BHR_LENGTH = 34;
    // static const int THRESHOLD = 79;
    // static const int WEIGHT_BITS = 7;
    // static const int MAX_ABS_WEIGHT = 64;
    // static const int ENTRY_SIZE = WEIGHT_BITS*BHR_LENGTH;
    // static const std::size_t PHT_SIZE = (std::size_t((64*1024)/ENTRY_SIZE));


    static const int BHR_LENGTH = 15;
    static const int THRESHOLD = 25;
    static const int WEIGHT_BITS = 7;
    static const int MAX_ABS_WEIGHT = 64;
    static const int ENTRY_SIZE = WEIGHT_BITS*BHR_LENGTH;
    static const std::size_t PHT_SIZE = (std::size_t((32*1024)/ENTRY_SIZE));


    // static const std::size_t PHT_SIZE = (std::size_t(1000));
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    // static const std::size_t PHT_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT_INDEX_MASK = (PHT_SIZE - 1);
    // static const counter_t PHT_INIT = /* weakly taken */ 2;
    int DOT_PRODUCT;
    
    typedef struct pht_entry
    {
        int weight[BHR_LENGTH+1];
    } pht_entry;
    
    static const pht_entry PHT_INIT;

    history_t bhr;                // 15 bits
    std::vector<pht_entry> pht;   // 64K bits 8K bytes

    void update_bhr(bool taken) { bhr >>= 1; if (taken) bhr |= BHR_MSB; }

    static std::size_t pht_index(address_t pc, history_t bhr) 
        { return (static_cast<std::size_t>(pc) & PHT_INDEX_MASK); }

  public:
    // PREDICTOR(void) : bhr(0), pht(PHT_SIZE, counter_t(PHT_INIT)) { }
    PREDICTOR(void) : bhr(0){
        // std::cout<<"PHT_SIZE "<<PHT_SIZE<<std::endl;
        pht.reserve(PHT_SIZE);
        for (uint i = 0; i < PHT_SIZE; ++i)
        {
            for(int j=0; j<BHR_LENGTH+1;j++)
                pht[i].weight[j] = 0;
        }
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
                address_t pc = br->instruction_addr;
                std::size_t index = pht_index(pc, bhr);
                int output = pht[index].weight[0];
                int x=1;
                for (int i = 0; i < BHR_LENGTH; ++i)
                {
                    int t = bhr>>(BHR_LENGTH-i-1);
                    t = (t&x);
                    if(t==1)
                        output += pht[index].weight[i+1];
                    else 
                        output -= pht[index].weight[i+1];
                }
                DOT_PRODUCT = output;
                if(output>=0)prediction = true;
                else prediction = false;
                // counter_t cnt = pht[index];
                // prediction = counter_msb(cnt);
            }
            return prediction;   // true for taken, false for not taken
        }

    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {
            if (/* conditional branch */ br->is_conditional) {
                // bool prediction = get_prediction(br, os);
                address_t pc = br->instruction_addr;
                std::size_t index = pht_index(pc, bhr);
                int output = pht[index].weight[0];
                int x = 1;
                for (int i = 0; i < BHR_LENGTH; ++i)
                {
                    int t = bhr>>(BHR_LENGTH-i-1);
                    t =(t&x);
                    if(t==1)
                        output += pht[index].weight[i+1];
                    else 
                        output -= pht[index].weight[i+1];
                }
                
                bool prediction;
                if(output>=0)prediction = true;
                else prediction = false;

                if(taken != prediction || abs(output)<=THRESHOLD){
                    address_t pc = br->instruction_addr;
                    std::size_t index = pht_index(pc, bhr);
                    for (int i = 0; i < BHR_LENGTH; ++i)
                    {
                        // if(abs(pht[index].weight[i+1])==MAX_ABS_WEIGHT)continue;
                        if(taken){
                            int t = bhr>>(BHR_LENGTH-i-1);
                            t = (t&x);
                            if(t==1){
                                if(pht[index].weight[i+1]==MAX_ABS_WEIGHT)continue;
                                pht[index].weight[i+1] += 1;
                            }
                            else{
                                if(pht[index].weight[i+1]==-MAX_ABS_WEIGHT)continue;
                                pht[index].weight[i+1] -= 1;
                            }
                        }else{
                            int t = bhr>>(BHR_LENGTH-i-1);
                            t = (t&x);
                            if(t==1){
                                if(pht[index].weight[i+1]==-MAX_ABS_WEIGHT)continue;
                                pht[index].weight[i+1] -= 1;
                            }
                            else{
                                if(pht[index].weight[i+1]==MAX_ABS_WEIGHT)continue;
                                pht[index].weight[i+1] += 1;  
                            }
                        }
                    }
                }
            update_bhr(taken);
            }
        }
};

#endif // PREDICTOR_H_SEEN

