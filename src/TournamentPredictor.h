#include <stdint.h>
#include "SaturationCounter.h"

#ifndef _TOURNAMENT_PREDICTOR_H_
#define _TOURNAMENT_PREDICTOR_H_

class BranchHistory
{
// Class to record the Branch History
public:
	BranchHistory(uint8_t hist_length = 10)
	{
        if (hist_length > 20 || hist_length <= 0)
        {
            hist_length = 10;
        }
        history = 0x0;
        mask = (1 << hist_length) - 1;
	}
	uint16_t getHistory()
	{
        return history;
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
// Class to manage local history 
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
        // Get prediction
        uint32_t mask_address = address & MASK_VALUE;
        uint32_t scindex = history[mask_address].getHistory();

        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
        uint32_t mask_address = address & MASK_VALUE;
        uint32_t scindex = history[mask_address].getHistory();

        if (outcome)    // taken
            ++counter[scindex];
        else            // not taken
            --counter[scindex];

        history[mask_address].updateHistory(outcome);

        return;
    }
private:
    static const uint8_t HISTORY_BITS = 10;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    static const uint32_t MASK_VALUE = HISTORY_SIZE - 1;
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
            counter[i] = SaturationCounter(2, 2);
    }
    bool shouldBranch(BranchHistory ghistory)
    {
        // Get prediction
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
    static const uint32_t HISTORY_BITS = 12;
    static const uint32_t HISTORY_SIZE = 1 << HISTORY_BITS;
    SaturationCounter counter[HISTORY_SIZE];
};


class TournamentPredictor
{
// Implements the alpha predictor
public:
    TournamentPredictor()
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
        // Remember the address for updating

        return (choose_global ? global_prediction : local_prediction);
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
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


// vim: ts=4 et sw=4:
#endif //_TOURNAMENT_PREDICTOR_H_

