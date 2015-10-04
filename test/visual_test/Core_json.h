#ifndef _CORE_H_
#define _CORE_H_

#include "utils.h"
#include <fstream>

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
	 * whiteToPlay - move turn
	 */

int leaf;
	
	MoveInfo think(PositionState& pos, uint16_t depth, bool whiteToPlay);

	Core();
    
	~Core();
  
private:
	/*
	 * It implements naive minimax algorithm (later add alpha-beta pruning)
	 */
	int16_t minimax(PositionState& pos, uint16_t depth, bool whiteToPlay);

   
	Core(const Core&); //non copy constructable
	const Core& operator=(const Core&); // non assignable
	
	PositionEvaluation* _posEval;
	TranspositionTable* _transTable;

  std::ofstream file;

};

}

#endif //_CORE_H_

