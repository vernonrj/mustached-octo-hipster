/* Author: Vernon Jones, Tyler Tricker;   
 * Description: This file defines the two required functions for the branch predictor.
*/

#include <vector>
#include <stdint.h>
#include "predictor.h"

// tuning parameters


// Static local datastructures. 

// forward declarations

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
    /* replace this code with your own */
    bool prediction = false;

    printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
           br->branch_target,br->is_indirect,br->is_conditional,
           br->is_call,br->is_return);
    if (br->is_conditional)
        prediction = true;
    return prediction;   // true for taken, false for not taken
}


// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
    /* replace this code with your own */
    printf("%1d\n",taken);

}


// private classes
class SaturationCounter
{
public:
    SaturationCounter(uint8_t numofbits, uint8_t initialValue = 0)
    {
        // Sanity Check parameters
        if((numofbits >= 8) || (numofbits < 1))
            numofbits = 1;

        if(initialValue > (1 << numofbits))
            m_CounterValue = 1;

        m_SizeInBits = numofbits;
 
    }
    /**
    * Ripple Counter Implementation
    */
    SaturationCounter& operator++()
    {
        if(m_CounterValue < (1 << m_SizeInBits)) 
            ++m_CounterValue;
        return *this;
    }

    /**
    * Decrement Counter until it saturates to a minimum
    */
    SaturationCounter& operator--()
    {
        if(m_CounterValue != 0)
            --m_CounterValue;
        return *this;
    }

    uint8_t GetMemoryUsage()
    {
        return m_SizeInBits;
    }

private:
    static const uint64_t COUNTERMAX = 255;
    uint8_t m_CounterValue;
    uint8_t m_SizeInBits;
};

// Static 'helper' functions - all functions below should be prefixed with 'static'


