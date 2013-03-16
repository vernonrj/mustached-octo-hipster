/* Author: Mark Faust
 *
 * C version of predictor file
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <cstring>
#include <inttypes.h>
#include "circular_stack.h"
#include "TournamentPredictor.h"
#include "hashtable.h"
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class

typedef CircularStack<uint32_t> Callstack;

class PREDICTOR
{
public:
    PREDICTOR();  // Default Constructor
    ~PREDICTOR(); // Default Destructor
    bool get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);

    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);
private:
    TournamentPredictor m_TournamentPredictor;
    HashTable<uint, uint> m_RelativePredictorTable;
    HashTable<uint, uint> m_AbsolutePredictorTable;
    CircularStack<uint> m_callstack;


};

#endif // PREDICTOR_H_SEEN

