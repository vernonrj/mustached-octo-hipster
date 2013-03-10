#include <stdint.h>
#include "SaturationCounter.h"


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
		unsigned entry:12;
	} history;
};


class LocalHistory
{
// Class to manage local history 
public:
    LocalHistory()
    {
        for (uint32_t i=0; i<HISTORY_SIZE; i++)
        {
            counter[i] = SaturationCounter(3, 3);
            history[i] = BranchHistory();
        }
    }
    bool shouldBranch(uint32_t address)
    {
        // Get prediction
        uint32_t mask_value = (1 << (HISTORY_BITS+1)) - 1;
        //uint32_t mask_address = address & 0x3FF;
        uint32_t mask_address = address & mask_value;
        uint32_t scindex = history[mask_address].getHistory();
        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(uint32_t address, uint8_t outcome)
    {
        uint32_t mask_value = (1 << (HISTORY_BITS+1)) - 1;
        uint32_t mask_address = address & mask_value;
        uint32_t scindex = history[mask_address].getHistory();
        if (outcome)    // taken
            ++counter[scindex];
        else            // not taken
            --counter[scindex];
    }
private:
    static const uint8_t HISTORY_BITS = 10;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    BranchHistory history[HISTORY_SIZE];
    SaturationCounter counter[HISTORY_SIZE];
};


class GlobalHistory
{
// class to manage global history
public:
    GlobalHistory()
    {
        ghistory = BranchHistory();
        for (uint32_t i=0; i<HISTORY_SIZE; i++)
            counter[i] = SaturationCounter(2, 1);
    }
    bool shouldBranch()
    {
        // Get prediction
        uint32_t scindex = ghistory.getHistory();
        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(uint8_t outcome)
    {
        uint32_t scindex = ghistory.getHistory();
        if (outcome)
            ++counter[scindex];
        else
            --counter[scindex];
        ghistory.updateHistory(outcome);
        return;
    }
private:
    static const uint32_t HISTORY_BITS = 12;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    BranchHistory ghistory;
    SaturationCounter counter[HISTORY_SIZE];
};


class TournamentPredictor
{
// class to manage global and local history
public:
    TournamentPredictor()
    {
        ghistory = GlobalHistory();
        lhistory = LocalHistory();
        tourn_hist = GlobalHistory();
        instruction_addr = 0x0;
    }
    bool shouldBranch(uint32_t address)
    {
        uint32_t word_address = address;
        bool lpredict = lhistory.shouldBranch(word_address);
        bool gpredict = ghistory.shouldBranch();
        bool choose_global = tourn_hist.shouldBranch();
        instruction_addr = address;
        if (choose_global)
            return gpredict;
        else
            return lpredict;
    }
    void updatePredictor(uint8_t outcome)
    {
        // check what we predicted
        uint32_t word_address = instruction_addr;
        bool lpredict = lhistory.shouldBranch(word_address);
        bool gpredict = ghistory.shouldBranch();
        bool choose_global = tourn_hist.shouldBranch();
        bool predicted_taken = shouldBranch(instruction_addr);
        instruction_addr = 0x0;

        lhistory.updatePredictor(word_address, outcome);
        ghistory.updatePredictor(outcome);
        if (lpredict == gpredict)
            return;

        // update tournament predictor
        if (outcome == predicted_taken)
        {
            // outcome was predicted correctly
            // unused predictor mispredicted
            if (choose_global)
                tourn_hist.updatePredictor(true);
            else
                tourn_hist.updatePredictor(false);
        }
        else
        {
            // outcome was not predicted correctly
            // unused predictor correctly predicted
            if (choose_global)
                tourn_hist.updatePredictor(false);
            else
                tourn_hist.updatePredictor(true);
        }
        return;
    }
private:
    GlobalHistory ghistory;
    LocalHistory lhistory;
    GlobalHistory tourn_hist;
    uint32_t instruction_addr;
};


