#ifndef _CORE_H_
#define _CORE_H_

#include "utils.h"

namespace pismo
{
class PositionState;
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
	int16_t minimax(PositionState& pos, uint16_t depth, bool white_to_play);

   
	Core(const Core&); //non copy constructable
	const Core& operator=(const Core&); // non assignable
	
	PositionEvaluation* _pos_eval;
	TranspositionTable* _trans_table;

};

}

#endif //_CORE_H_

