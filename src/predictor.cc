/* Author: Vernon Jones, Tyler Tricker;   
 * Description: This file defines the two required functions for the branch predictor.
*/

#include <stdint.h>
#include <stdlib.h>
#include "SaturationCounter.h"
#include "predictor.h"

// tuning parameters

// forward declarations
static int getenvironmentint(const char* env_name, int defaultvalue);
// Static local datastructures. 



PREDICTOR::PREDICTOR()
{
    //get environment variables to setup cache - remember to keep track of mem usage
    const char* predictor_callstack_size = getenv("predictor_callstack_size");
    m_callstack.resize(getenvironmentint("predictor_callstack_size", 4));
}

PREDICTOR::~PREDICTOR()
{

}

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
    printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
           br->branch_target,br->is_indirect,br->is_conditional,
           br->is_call,br->is_return);

    if (br->is_call)
    {
        //push address onto stack
        return true;
    }
    else if (br->is_return)
    {
        //pop address from stack
        return true;
    }
    else if (br->is_conditional)
    {
        // TODO alpha predictor
        return true;
    }
    // instruction not branch
    return false;
}


// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(
    const branch_record_c* br, 
    const op_state_c* os, 
    bool taken)
{
    /* replace this code with your own */
    printf("%1d\n",taken);

}


// private classes

class LocalHistory
{
public:
    LocalHistory()
    {
        for (int i=0; i<1024; i++)
            counter[i] = SaturationCounter(3, 2);
    }
    bool shouldBranch(uint32_t address)
    {
        uint32_t scindex = history[address].entry;
        return counter[scindex]() < (counter[scindex].GetCounterValue() >> 1);
    }
private:
    struct history_t
    {
        unsigned entry:10;

    } history[1024];
    SaturationCounter counter[1024];
};

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


