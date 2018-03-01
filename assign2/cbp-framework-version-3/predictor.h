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

class PREDICTOR
{
  public:
    typedef uint32_t address_t;

  private:
    typedef uint32_t history_t;
    typedef uint8_t counter_t;

    static const int BHR_LENGTH = 14;
    // never changes. Used to add 1 in the bhr by ORing.
    static const history_t BHR1_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const history_t BHR2_MSB = (history_t(1) << (BHR_LENGTH - 2));
    static const history_t BHR3_MSB = (history_t(1) << (BHR_LENGTH - 2));
    static const std::size_t PHT1_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT2_SIZE = (std::size_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PHT3_SIZE = (std::size_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PHT1_INDEX_MASK = (PHT1_SIZE - 1);
    static const std::size_t PHT2_INDEX_MASK = (PHT2_SIZE - 1);
    static const std::size_t PHT3_INDEX_MASK = (PHT3_SIZE - 1);
    static const counter_t PHT_INIT = /* weakly taken */ 2;

    history_t bhr1;                // 15 bits
    history_t bhr2;                // 15 bits
    history_t bhr3;                // 15 bits
    std::vector<counter_t> pht1;   // 64K bits
    std::vector<counter_t> pht2;   // 64K bits
    std::vector<counter_t> pht3;   // 64K bits

    void update_bhr(bool taken) { 
        bhr1 >>= 1; if (taken) bhr1 |= BHR1_MSB;
        bhr2 >>= 1; if (taken) bhr2 |= BHR2_MSB;
        bhr3 >>= 1; if (taken) bhr3 |= BHR3_MSB;
    }
    static std::size_t pht1_index(address_t pc, history_t bhr) 
        { return (static_cast<std::size_t>(pc ^ bhr) & PHT1_INDEX_MASK); }
    static std::size_t pht2_index(address_t pc, history_t bhr) 
        { return (static_cast<std::size_t> (pc & PHT2_INDEX_MASK)); }
    static std::size_t pht3_index(address_t pc, history_t bhr) 
        { return (static_cast<std::size_t> (bhr & PHT3_INDEX_MASK)); }
    //Assuming 2 bit counter
    static bool counter_msb(/* 2-bit counter */ counter_t cnt) { return (cnt >= 2); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
        { if (cnt != 3) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
        { if (cnt != 0) --cnt; return cnt; }

  public:
    PREDICTOR(void) : bhr1(0), bhr2(0), bhr3(0), pht1(PHT1_SIZE, counter_t(PHT_INIT)), pht2(PHT2_SIZE, counter_t(PHT_INIT)), pht3(PHT3_SIZE, counter_t(PHT_INIT)) { }
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
                std::size_t index1 = pht1_index(pc, bhr1);
                std::size_t index2 = pht2_index(pc, bhr2);
                std::size_t index3 = pht3_index(pc, bhr3);
                counter_t cnt1 = pht1[index1];
                counter_t cnt2 = pht2[index2];
                counter_t cnt3 = pht3[index3];
                counter_t cntf = counter_msb(cnt1) + counter_msb(cnt2) + counter_msb(cnt3);
                prediction = counter_msb(cntf);
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
                std::size_t index1 = pht1_index(pc, bhr1);
                std::size_t index2 = pht2_index(pc, bhr2);
                std::size_t index3 = pht3_index(pc, bhr3);
                counter_t cnt1 = pht1[index1];
                counter_t cnt2 = pht2[index2];
                counter_t cnt3 = pht3[index3];
                if (taken)
                {
                    cnt1 = counter_inc(cnt1);
                    cnt2 = counter_inc(cnt2);
                    cnt3 = counter_inc(cnt3);
                }
                else
                {
                    cnt1 = counter_dec(cnt1);
                    cnt2 = counter_dec(cnt2);
                    cnt3 = counter_dec(cnt3);
                }
                pht1[index1] = cnt1;
                pht2[index2] = cnt2;
                pht3[index3] = cnt3;
                update_bhr(taken);
            }
        }
};

#endif // PREDICTOR_H_SEEN

