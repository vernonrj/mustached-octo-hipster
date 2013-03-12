/* Author: Vernon Jones, Tyler Tricker;   
 * Description: This file defines the two required functions for the branch predictor.
*/

#include <stdint.h>
#include <stdlib.h>
#include "SaturationCounter.h"
#include "TournamentPredictor.h"
#include "predictor.h"

// tuning parameters

// forward declarations
static int getenvironmentint(const char* env_name, int defaultvalue);
static uint hash (uint address);

// Static local datastructures. 


PREDICTOR::PREDICTOR()
: m_RelativePredictorTable(hash),
m_AbsolutePredictorTable(hash)
{
    //get environment variables to setup cache - remember to keep track of mem usage
    m_callstack.resize(getenvironmentint("predictor_callstack_size", 16));
}

PREDICTOR::~PREDICTOR()
{

}

bool PREDICTOR::get_prediction(
    const branch_record_c* br, 
    const op_state_c* os, 
    uint *predicted_target_address)
{
/*    printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
           *predicted_target_address,br->is_indirect,br->is_conditional,
           br->is_call,br->is_return);
*/

    if (br->is_call)
    {
        //push address onto stack
        *predicted_target_address = m_AbsolutePredictorTable[br->instruction_addr];
        m_callstack.push(br->instruction_next_addr);
        return true;

    }
    else if (br->is_return)
    {
        //pop address from stack
        *predicted_target_address = m_callstack.pop();
        return true;
    }
    else if (br->is_conditional)
    {
//      printf("%1d ", m_TournamentPredictor.shouldBranch(br->instruction_addr));
        bool branchTaken = m_TournamentPredictor.shouldBranch(br->instruction_addr);

        *predicted_target_address = (br->is_indirect) ?
            m_RelativePredictorTable[br->instruction_addr]:
            m_AbsolutePredictorTable[br->instruction_addr];

        return branchTaken;
    }
    else 
    {
        // instruction not branch
        *predicted_target_address = br->instruction_next_addr;
        return false;
    }

}


// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(
    const branch_record_c* br, 
    const op_state_c* os, 
    bool taken, 
    uint actual_target_address)
{
    // Test misses and report them
    if(br->is_conditional)
    {
        uint predicted_addr = (br->is_indirect) ?
           m_RelativePredictorTable[br->instruction_addr]:
           m_AbsolutePredictorTable[br->instruction_addr];

        if (predicted_addr != actual_target_address)
        {
            static int count = 0;
            printf("Conditional :%d\n", ++count);
        }

    }

    //update tables and predictors
    if(br->is_call)
    {
        m_AbsolutePredictorTable[br->instruction_addr] = actual_target_address;
    } else if(br->is_return)
    {

    } else if(br->is_conditional)
    {
        if(br->is_indirect)
            m_RelativePredictorTable[br->instruction_addr] = actual_target_address;
        else
            m_AbsolutePredictorTable[br->instruction_addr] = actual_target_address;

        m_TournamentPredictor.updatePredictor(taken);
    }

<<<<<<< HEAD
=======
    /* replace this code with your own */
    //printf("%1d\n",taken);
    if (br->is_conditional)
        m_TournamentPredictor.updatePredictor(br->instruction_addr, taken);
>>>>>>> 21e6c67f2b177d62517faf2342a04b91fa06374f
}

// Static 'helper' functions - all functions below should be prefixed with 'static'
static int getenvironmentint(const char* env_name, int defaultvalue = 0)
{
    const char* env_variable = getenv(env_name);
    int integer = defaultvalue;
    if(env_variable)
    {
       sscanf(env_variable, "%d", &integer);
    }

    return integer;
}

// Dummy Hash function for a baseline
static uint hash (uint address)
{
    return address;
}

// vim: ts=4 et sw=4:

