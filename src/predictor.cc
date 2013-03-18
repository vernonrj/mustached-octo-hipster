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
static std::string getenvironmentstr(const char* env_name, const char* defaultvalue);

// Static local variables. 
PREDICTOR::PREDICTOR()
{
    std::string outputfilename = getenvironmentstr("logfile", "log.txt");
    m_outputfile.open(outputfilename.c_str(), std::fstream::out);
    m_outputfile << "InstructionAddress" << ',' 
                 << "Target"  << ","
                 << "Taken"   << ","
                 << "Conditional" << ","
                 << "Indirect"  << ","
                 << "Call"      << ","
                 << "Return" << "\n";
}

PREDICTOR::~PREDICTOR()
{
    m_outputfile.close(); 
}

bool PREDICTOR::get_prediction(
    const branch_record_c* br, 
    const op_state_c* os, 
    uint *predicted_target_address)
{
   return false;
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

    m_outputfile << br->instruction_addr << ',' 
                 << actual_target_address << ','
                 << taken << ','
                 << br->is_conditional << ','
                 << br->is_indirect << ','
                 << br->is_call << ','
                 << br->is_return << '\n';

}

static std::string getenvironmentstr(const char* env_name, const char* defaultvalue)
{
    const char* env_variable = getenv(env_name);
    if(env_variable)
    {
        return std::string(env_variable);
    } else
    {
        return std::string(defaultvalue);
    }
}

// vim: ts=4 et sw=4:

