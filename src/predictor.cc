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
    m_callstack.resize(getenvironmentint("predictor_callstack_size", 4));
    m_callstack.push(5); //test stack
}

PREDICTOR::~PREDICTOR()
{

}

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
        return true;
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

}


// private classes



class BranchHistory
{
public:
	BranchHistory()
	{
		history.entry = 0x0;
	}
	uint16_t getHistory()
	{
		// Should we return this,
		// or use inheritance?
		return history.entry;
	}
	void updateHistory(uint8_t new_entry)
	{
		history.entry = history.entry << 1;
		history.entry |= (new_entry & 0x1);
		return;
	}
private:
	struct history_t
	{
		unsigned entry:10;
	} history;
};


class LocalHistory
{
// Class to manage local history 
public:
    LocalHistory()
    {
        for (int i=0; i<1024; i++)
	{
        counter[i] = SaturationCounter(3, 2);
	    history[i] = BranchHistory();
	}
    }
    bool shouldBranch(uint32_t address)
    {
	// Get prediction
        uint32_t mask_address = address & 0x3FF;
        uint32_t scindex = history[mask_address].getHistory();
        return counter[scindex]() < (counter[scindex].GetCounterValue() >> 1);
    }
    void updatePredictor(uint32_t address, uint8_t outcome)
    {
        uint32_t mask_address = address & 0x3FF;
        uint32_t scindex = history[mask_address].getHistory();
        if (outcome)    // taken
            ++counter[scindex];
        else            // not taken
            --counter[scindex];
    }
private:
    BranchHistory history[1024];
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


