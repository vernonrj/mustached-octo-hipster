#ifndef _SATURATION_COUNTER_H_
#define _SATURATION_COUNTER_H_

#include<stdint.h>

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

    uint8_t GetCounterValue()
    {
        return m_CounterValue;
    }

private:
    static const uint64_t COUNTERMAX = 255;
    uint8_t m_CounterValue;
    uint8_t m_SizeInBits;
};

#else
#endif
