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
static uint hash (uint address);

// Static local variables. 
static uint s_PreviousTargetPrediction = 0;
static bool s_PreviousBranchPrediction = false;

static uint s_ConditionalMissesB        = 0;
static uint s_CallMissesB               = 0;
static uint s_IndirectMissesB           = 0;
static uint s_ReturnMissesB             = 0;
static uint s_GenericMissB              = 0;


static uint s_ConditionalMisses        = 0;
static uint s_CallMisses               = 0;
static uint s_IndirectMisses           = 0; 
static uint s_ReturnMisses             = 0;
static uint s_GenericMiss              = 0;


PREDICTOR::PREDICTOR()
: m_RelativePredictorTable(hash),
m_AbsolutePredictorTable(hash)
{
    //get environment variables to setup cache - remember to keep track of mem usage
    m_callstack.resize(getenvironmentint("predictor_callstack_size", 16));
}

PREDICTOR::~PREDICTOR()
{
    printf("Target Stats:\n");
    printf("     CallMisses  : %d\n", s_CallMisses);
    printf("     ReturnMisses: %d\n", s_ReturnMisses );
    printf("     ConBranchMis: %d\n", s_ConditionalMisses);
    printf("     IndirectMiss: %d\n", s_IndirectMisses);
    printf("     GenericMisse: %d\n", s_GenericMiss);

    printf("Prediction Stats:\n");
    printf("     CallMisses  : %d\n", s_CallMisses);
    printf("     ReturnMisses: %d\n", s_ReturnMisses );
    printf("     ConBranchMis: %d\n", s_ConditionalMisses);
    printf("     IndirectMiss: %d\n", s_IndirectMisses);
    printf("     GenericMisse: %d\n", s_GenericMiss);
}

bool PREDICTOR::get_prediction(
    const branch_record_c* br, 
    const op_state_c* os, 
    uint *predicted_target_address)
{

    bool branchTaken = false;

    if (br->is_call)
    {
        //push address onto stack
        *predicted_target_address = m_AbsolutePredictorTable[br->instruction_addr];
        m_callstack.push(br->instruction_next_addr);
        branchTaken = true;
    }
    else if (br->is_return)
    {
        //pop address from stack
        *predicted_target_address = m_callstack.pop();
        branchTaken = true;
    }
    else if (br->is_conditional)
    {
        branchTaken = m_TournamentPredictor.shouldBranch(br->instruction_addr);

        *predicted_target_address = (br->is_indirect) ?
            m_RelativePredictorTable[br->instruction_addr]:
            m_AbsolutePredictorTable[br->instruction_addr];
    }
    else 
    {
        // Unconditional Branch

        *predicted_target_address = br->instruction_next_addr;
        branchTaken = true;
    }

    s_PreviousTargetPrediction = *predicted_target_address; //Statistic gathering
    s_PreviousBranchPrediction = branchTaken;               //Statistic gathering
 
    return branchTaken;
}

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(
    const branch_record_c* br, 
    const op_state_c* os, 
    bool taken, 
    uint actual_target_address)
{

    //Miss Detection
    //Test for prediction miss
    if(s_PreviousBranchPrediction != taken)
    {
        s_CallMissesB   = br->is_call   ? s_CallMissesB + 1: s_CallMissesB;
        s_ReturnMissesB = br->is_return ? s_ReturnMissesB + 1: s_ReturnMissesB;
        s_ConditionalMissesB = br->is_conditional ? s_ConditionalMissesB + 1: s_ConditionalMissesB;
        s_IndirectMissesB = br->is_indirect ? s_IndirectMissesB + 1: s_IndirectMisses;
        ++s_GenericMissB;
    } 

    //Test for miss target for statistics
    if(s_PreviousTargetPrediction != actual_target_address)
    {
        s_CallMisses   = br->is_call   ? s_CallMisses + 1: s_CallMisses;
        s_ReturnMisses = br->is_return ? s_ReturnMisses + 1: s_ReturnMisses;
        s_ConditionalMisses = br->is_conditional ? s_ConditionalMisses + 1: s_ConditionalMisses;
        s_IndirectMisses = br->is_indirect ? s_IndirectMisses + 1: s_IndirectMisses;
        ++s_GenericMiss;
    }

    //update tables and predictors
    if(br->is_call)
    {
        m_AbsolutePredictorTable[br->instruction_addr] = actual_target_address;
    } else if(br->is_return)
    {

    } else if(br->is_return)
    {

    } else (br->is_conditional)
    {
        if(br->is_indirect)
            m_RelativePredictorTable[br->instruction_addr] = actual_target_address;
        else
            m_AbsolutePredictorTable[br->instruction_addr] = actual_target_address;

        m_TournamentPredictor.updatePredictor(br->instruction_addr, taken);
    }

}

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

// Dummy Hash function for a baseline
static uint hash (uint address)
{
    return address;
}

// vim: ts=4 et sw=4:

