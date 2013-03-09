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
        for (int i=0; i<1024; i++)
        {
            counter[i] = SaturationCounter(3, 4);
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


class GlobalHistory
{
// class to manage global history
public:
    GlobalHistory()
    {
        int i;
        ghistory = BranchHistory();
        for (i=0; i<4096; i++)
            counter[i] = SaturationCounter(2, 2);
    }
    bool shouldBranch()
    {
        // Get prediction
        uint32_t scindex = ghistory.getHistory();
        return counter[scindex]() < (counter[scindex].GetCounterValue() >> 1);
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
    BranchHistory ghistory;
    SaturationCounter counter[4096];
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
    }
    bool shouldBranch(uint32_t address)
    {
        bool lpredict = lhistory.shouldBranch(address);
        bool gpredict = ghistory.shouldBranch();
        bool choose_global = tourn_hist.shouldBranch();
        if (choose_global)
            return gpredict;
        else
            return lpredict;
    }
    void updatePredictor(uint32_t address, uint8_t outcome)
    {
        bool lpredict = lhistory.shouldBranch(address);
        bool gpredict = ghistory.shouldBranch();
        bool choose_global = tourn_hist.shouldBranch();
        bool predicted_taken = shouldBranch(address);

        lhistory.updatePredictor(address, outcome);
        ghistory.updatePredictor(outcome);
        if (lpredict == gpredict)
            return;

        // update tournament predictor
        if (outcome == predicted_taken)
        {
            // outcome was predicted correctly
            if (choose_global)
                tourn_hist.updatePredictor(true);
            else
                tourn_hist.updatePredictor(false);
        }
        else
        {
            // outcome was not predicted correctly
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
};


