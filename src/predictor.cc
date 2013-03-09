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
// Static local datastructures. 



PREDICTOR::PREDICTOR()
{
    //get environment variables to setup cache - remember to keep track of mem usage
    m_callstack.resize(getenvironmentint("predictor_callstack_size", 4));
    m_callstack.push(5); //test stack
}

PREDICTOR::~PREDICTOR()
{

}

TournamentPredictor tpredict = TournamentPredictor();

//bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{
    printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
           *predicted_target_address,br->is_indirect,br->is_conditional,
           br->is_call,br->is_return);

    if (br->is_call)
    {
        //push address onto stack
        m_callstack.push(*predicted_target_address);
        return true;
    }
    else if (br->is_return)
    {
        //pop address from stack
        // *address = m_callstack.pop();
        m_callstack.pop();
        return true;
    }
    else if (br->is_conditional)
    {
        // TODO alpha predictor
        return tpredict.shouldBranch(br->instruction_addr);
        //return true;
    }
    // instruction not branch
    return false;
}


// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
//void PREDICTOR::update_predictor(
//    const branch_record_c* br, 
//   const op_state_c* os, 
//    bool taken)
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{
    /* replace this code with your own */
    printf("%1d\n",taken);
    tpredict.updatePredictor(actual_target_address, taken);
}


// private classes





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


