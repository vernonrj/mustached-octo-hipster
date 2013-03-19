/* Author: Mark Faust
 *
 * C version of predictor file
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <tr1/functional>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <inttypes.h>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class


class SaturationCounter
{
public:
    SaturationCounter(uint8_t numofbits = 2, uint8_t initialValue = 0)
    {
        this->ChangeNumofBits(numofbits, initialValue); 
    }

    void ChangeNumofBits(uint8_t numofbits, uint8_t initialValue)
    {
        
        // Sanity Check parameters
        if((numofbits >= 8) || (numofbits < 1))
            numofbits = 1;

        if(initialValue > (1 << numofbits))
            m_CounterValue = 1;
        else
            m_CounterValue = initialValue;

        m_SizeInBits = numofbits;
    }

    /**
    * Increment Counter unless it saturates to its maximum value 
    */
    SaturationCounter& operator++()
    {
        if(m_CounterValue < (1 << m_SizeInBits)) 
            ++m_CounterValue;
        return *this;
    }

    /**
    * Decrement Counter unless it saturates to its minimum value 
    */
    SaturationCounter& operator--()
    {
        if(m_CounterValue != 0)
            --m_CounterValue;
        return *this;
    }

    /**
    * Functor operator gets the counter value 
    */
    uint8_t operator() ()
    {
       return this->GetCounterValue();
    }

    uint8_t GetMemoryUsage()
    {
        return m_SizeInBits;
    }

    uint8_t GetCounterValue()
    {
        return m_CounterValue;
    }

    uint8_t GetCounterMax()
    {
        return (1 << m_SizeInBits);
    }

private:
    static const uint64_t COUNTERMAX = 255;
    uint8_t m_CounterValue;
    uint8_t m_SizeInBits;
};





// The Alpha Predictor
// Uses a Tournament Predictor to choose between
// a Global Predictor
// and a Local Predictor

class BranchHistory
{
// Records the Branch History
public:
	BranchHistory(uint8_t hist_length = 10)
	{
        if (hist_length > 20 || hist_length <= 0)
            hist_length = 10;   // Sanity Check
        history = 0x0;
        // Generate a mask to hold history at
        // a specified length
        mask = (1 << hist_length) - 1;
	}
	uint16_t getHistory()
	{
        return (history & mask);
	}
	void updateHistory(bool new_entry)
	{
        history = (history << 1) & mask;
        history |= (new_entry & 0x1);
		return;
	}
private:
    uint32_t history;
    uint32_t mask;
};


class LocalHistory
{
// Class to manage a Local History Table
public:
    LocalHistory()
    {
        for (uint32_t i=0; i<HISTORY_SIZE; i++)
        {
            counter[i] = SaturationCounter(3, 4);
            history[i] = BranchHistory(HISTORY_BITS);
        }
    }
    bool shouldBranch(uint32_t address)
    {
        // Predict the branch outcome at PC address
        uint32_t mask_address = address & MASK_VALUE;
        uint32_t scindex = history[mask_address].getHistory();

        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
        uint32_t mask_address = address & MASK_VALUE;
        uint32_t scindex = history[mask_address].getHistory();

        // Update the local Saturating Counter
        if (outcome)    // taken
            ++counter[scindex];
        else            // not taken
            --counter[scindex];

        // Update the local Path History
        history[mask_address].updateHistory(outcome);

        return;
    }
private:
    // Size constants
    static const uint8_t HISTORY_BITS = 10;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    // Bounds Enforcement
    static const uint32_t MASK_VALUE = HISTORY_SIZE - 1;
    // Tables
    BranchHistory history[HISTORY_SIZE];
    SaturationCounter counter[HISTORY_SIZE];
};


class GlobalHistory
{
// class to manage global history
public:
    GlobalHistory()
    {
        for (uint32_t i=0; i<HISTORY_SIZE; i++)
            counter[i] = SaturationCounter(2, 1);
    }
    bool shouldBranch(BranchHistory ghistory)
    {
        // Predict the branch outcome based on the global path history
        uint32_t scindex = ghistory.getHistory();
        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(BranchHistory ghistory, bool outcome)
    {
        // Update Saturating Counter based on outcome
        uint32_t scindex = ghistory.getHistory();
        if (outcome)		// taken
            ++counter[scindex];
        else 			// not taken
            --counter[scindex];
        return;
    }
private:
    // Size constants
    static const uint32_t HISTORY_BITS = 12;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    // Table
    SaturationCounter counter[HISTORY_SIZE];
};


class AlphaPredictor
{
// Implements the alpha predictor
public:
    AlphaPredictor()
        :ghistory(GlobalHistory()),
         lhistory(LocalHistory()),
         tourn_hist(GlobalHistory()),
         path_history(BranchHistory(12))
    {}
    bool shouldBranch(uint32_t address)
    {
        // Test whether we should branch
        
        // Get predictions from local, global, and tournament
        bool local_prediction = lhistory.shouldBranch(address);
        bool global_prediction = ghistory.shouldBranch(path_history);
        bool choose_global = tourn_hist.shouldBranch(path_history);

        return (choose_global ? global_prediction : local_prediction);
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
        // Update All Predictors based on branch outcome at PC address
        
        // First recheck our predictions
        bool local_prediction = lhistory.shouldBranch(address);
        bool global_prediction = ghistory.shouldBranch(path_history);
        bool choose_global = tourn_hist.shouldBranch(path_history);
        bool predicted_taken = shouldBranch(address);

        // Update local and global predictors 
        lhistory.updatePredictor(address, outcome);
        ghistory.updatePredictor(path_history, outcome);

        if (local_prediction != global_prediction)
        {
            // Predictors predicted differently.
            // update tournament predictor
            // based on whether outcome was correctly predicted
            if (outcome == predicted_taken)
            {
                // outcome was predicted correctly
                // unused predictor mispredicted
                tourn_hist.updatePredictor(path_history, choose_global);
            }
            else
            {
                // outcome was not predicted correctly
                // unused predictor correctly predicted
                tourn_hist.updatePredictor(path_history, !choose_global);
            }
        }

        // Finally, update the path history
        path_history.updateHistory(outcome);

        return;
    }
private:
    GlobalHistory ghistory;         // Global History
    LocalHistory lhistory;          // Local History
    GlobalHistory tourn_hist;       // Tournament History
    BranchHistory path_history;     // Global Path History
};



// this is a dirty circular stack - it will return an old value or overwrite
// an old value if it runs out of bounds

template <typename T>
class CircularStack
{
   //local definitions

public:
   CircularStack()
   :m_top(0)
   {}

   virtual ~CircularStack() {}
   void resize(size_t newsize)
   {
       m_datavector.resize(newsize);
       m_top = (m_top < newsize) ? m_top: m_top % newsize;
   }

   /**
    * Push a new element onto the stack
    */
   void push(T newvalue)
   {
      size_t stacksize = m_datavector.size();
      m_top = (m_top >= stacksize) ? 0 : m_top + 1;
      m_datavector[m_top] = newvalue;
   }

   /**
    * Returns the element currently on the top of the stack.
    */
   T pop()
   {
      T topelement = m_datavector[m_top];
      m_top = (m_top == 0) ? m_datavector.size() : m_top - 1; 
      return topelement;
   }

   /** returns the memory budget of the stack in bits 
   *   (might want to refactor to allow for arbitrary bit sizes)
   */
   size_t memoryusage()
   {
      return sizeof(T)*8*m_datavector.size();
   }

private:
   std::vector<T> m_datavector;
   size_t m_top;
};




// This class implements a fully associative cache.
// LRU replacement is implemented with the clock algorithm.
class SetAssociativeCache
{
    //private forward declarations
    struct line_t;
    struct set_t;
    static void null_evict(uint,uint){}
public:
    SetAssociativeCache()
    :m_evictfn(null_evict)
    {}

    ~SetAssociativeCache()
    {
    }

    void setDimensions(uint sets, uint associativity=1)
    {
        using namespace std;
        m_storage.clear(); //clear out cache before resize
        m_storage.resize(sets);
        for(std::vector<set_t>::iterator i = m_storage.begin(); 
            i != m_storage.end(); ++i)
        {
            i->m_counter = 0;
            i->m_storage.resize(associativity);
        }
    }

    // A good use for this is to use bind to have the evict call into
    // a second level caches add. e.g.
    // cacheobject1.setEvictCallBack(
    //     std::tr1::bind(&Class::memfun, 
    //     &cacheobject2, _1, _2));
    // where memfun is a function that takes (uint, uint)
    void setEvictCallBack(std::tr1::function<void (uint, uint)> fn)
    {
        m_evictfn = fn;
    }

    // e.g. on use
    // cache.additem(0x123456, data, evicted_addr, evicted_data);
    void additem(uint addr, uint data)
    {
       using namespace std;
       uint tag = addrtotag(addr);

       vector<line_t>& set = m_storage[addrtoindex(addr)].m_storage; 
       uint& counter = m_storage[addrtoindex(addr)].m_counter;

       uint evaddress = 0;
       uint evdata = 0;

       vector<line_t>::iterator it; 
       it = find(set.begin(), set.end(), tag);
       if(it != set.end())
       {
           it->used = true;
           it->value = data;
           return;
       }
       else //find an element to evict
       {
           while(true)
           {
               //increment counter and get element reference
               counter = (counter + 1) % set.size();
               line_t& element = set[counter];

               //flip used bits
               element.used = ~element.used;
               if(element.used == true) // value to be evicted
               {
                   evaddress = (addr & (tag << log2(m_storage.size())));
                   evdata = element.value;
                   if(evaddress != 0)
                       m_evictfn(evaddress, evdata);

                   element.tag = tag;
                   element.value = data;
                   return;
               }
           }
       }
    }

    //data = cache.getitem(0x12345679);
    uint getitem(uint addr)
    {
       using namespace std;
       uint tag = addrtotag(addr);

       vector<line_t>& set = m_storage[addrtoindex(addr)].m_storage; 

       vector<line_t>::iterator it; 
       it = find(set.begin(), set.end(), tag);
       if(it != set.end()) // if tag is present just return a reference
       {   
           it->used = true;
           return it->value;
       }
        return 0;
    }
    //return the Ceil(log2(x))
    static uint log2(uint x)
    {
        uint i;
        for(i = 0; x >= ((unsigned)1<<i); ++i){}
        return i-1;
    }

    uint addrtotag(uint addr)
    {
       //calc index bit size
        uint indexbits = log2(m_storage.size());
       return addr >> indexbits;
    }

    virtual uint addrtoindex(uint addr)
    {
        //calc index bit size and make mask
        uint indexbits = log2(m_storage.size());
        uint mask = ~0 << indexbits;
        mask = ~mask;
        //mask out all other bits
        return addr & mask;
    }
private:
    struct set_t //set size = sizeof(line_t) + log2(ways)
    {
        std::vector<line_t> m_storage;
        uint m_counter;
    };

    struct line_t
    {
        uint tag;
        uint value;
        bool used;

        line_t() //line size = sizeof(value) + sizeof(tag) + 1
        {
            tag   = 0;
            value = 0;
            used  = false; //size 1
        }

        bool operator==(const uint x)
        {
           return (x == this->tag);
        }
    };

    std::vector<set_t> m_storage;
    std::tr1::function<void (uint,uint)> m_evictfn;
};

class PcRelCache: public SetAssociativeCache 
{
    virtual uint addrtotag(uint addr)
    {
        return addr;
    }

};




typedef CircularStack<uint32_t> Callstack;
static const uint RELATIVE_OFFSET = 1 << 10;

class PREDICTOR
{
public:
    PREDICTOR();  // Default Constructor
    ~PREDICTOR(); // Default Destructor
    bool get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);

    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);
private:
    AlphaPredictor m_AlphaPredictor;
    SetAssociativeCache m_BranchTargetTable;
    SetAssociativeCache m_PCRelTable;
    CircularStack<uint> m_callstack;


};

#endif // PREDICTOR_H_SEEN

