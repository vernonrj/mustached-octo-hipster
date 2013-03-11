#include <stdint.h>
#include "SaturationCounter.h"

#ifndef _TOURNAMENT_PREDICTOR_H_
#define _TOURNAMENT_PREDICTOR_H_

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
	void updateHistory(bool new_entry)
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
            counter[i] = SaturationCounter(3, 4);
            history[i] = BranchHistory();
        }
    }
    bool shouldBranch(uint32_t address)
    {
        // Get prediction
        uint32_t mask_value = (1 << HISTORY_BITS) - 1;
        //uint32_t mask_address = address & 0x3FF;
        uint32_t mask_address = address & mask_value;
        uint32_t scindex = history[mask_address].getHistory();
        return counter[scindex]() >= (counter[scindex].GetCounterMax() >> 1);
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
        uint32_t mask_value = (1 << HISTORY_BITS) - 1;
        uint32_t mask_address = address & mask_value;
        uint32_t scindex = history[mask_address].getHistory();
        if (outcome)    // taken
            ++counter[scindex];
        else            // not taken
            --counter[scindex];
        history[mask_address].updateHistory(outcome);
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
         path_history(BranchHistory())
    {}
    bool shouldBranch(uint32_t address)
    {
        // Test whether we should branch
        
        // debug variable; TODO: check to see if address is byte-aligned
        //instruction_addr = address;
        // Get predictions from local, global, and tournament
        bool local_prediction = lhistory.shouldBranch(address);
        bool global_prediction = ghistory.shouldBranch(path_history);
        bool choose_global = tourn_hist.shouldBranch(path_history);
        // Remember the address for updating

        return (choose_global ? global_prediction : local_prediction);
        //return predicted_taken;
    }
    void updatePredictor(uint32_t address, bool outcome)
    {
        // check what we predicted
        // First get what our predictions were
        BranchHistory old_history(path_history);

        bool local_prediction = lhistory.shouldBranch(address);
        bool global_prediction = ghistory.shouldBranch(old_history);
        bool choose_global = tourn_hist.shouldBranch(old_history);
        bool predicted_taken = shouldBranch(address);

        // Update local and global predictors 
        path_history.updateHistory(outcome);
        lhistory.updatePredictor(address, outcome);
        ghistory.updatePredictor(old_history, outcome);


        if (local_prediction == global_prediction)
        {
            // Both predictors predicted the same outcome.
            // Don't need to update the tournament predictor
            return;
        }

        // Predictors predicted differently.
        // update tournament predictor
        // based on whether outcome was correctly predicted
        if (outcome == predicted_taken)
        {
            // outcome was predicted correctly
            // unused predictor mispredicted
            tourn_hist.updatePredictor(old_history, choose_global);
        }
        else
        {
            // outcome was not predicted correctly
            // unused predictor correctly predicted
            tourn_hist.updatePredictor(old_history, !choose_global);
        }
        return;
    }
private:
    GlobalHistory ghistory;         // Global History
    LocalHistory lhistory;          // Local History
    GlobalHistory tourn_hist;       // Tournament History
    BranchHistory path_history;     // Global Path History
    // Branching Address, Prediction outcomes
};


// vim: ts=4 et sw=4:
#endif //_TOURNAMENT_PREDICTOR_H_

