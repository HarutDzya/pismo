#ifndef _CORE_H_
#define _CORE_H_

#include "PositionState.h"
#include "utils.h"

namespace pismo
{

class PositionEvaluation;
class TranspositionTable;

class Core
{
public:

    /** Returns the best move in current position.
     * pos - current position
     * depth - search depth
     * white_to_play - move turn
     */
    move_info think(PositionState& pos, uint16_t depth, bool white_to_play);

    Core();
    
    ~Core();
  
private:
    /*
     * It implements naive minimax algorithm (later add alpha-beta pruning)
     */
    float minimax(PositionState& pos, uint16_t depth, bool white_to_play);

    const PositionEvaluation* _pos_eval;
    TranspositionTable* _trans_table;

    const Core& Core(const Core&); //non copy constructable
    const Core& operator=(const Core&); // non assignable
};

}

#endif //_BRAIN_H_

