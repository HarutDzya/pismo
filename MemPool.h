#ifndef _MEM_POOL_
#define _MEM_POOL_

#include "utils.h"

namespace pismo
{

//reusing allocated memory to avoid wasting time on system calls (free(), alloc())


//maximum number of moves one side can have in current position
const int MAX_POSSIBLE_MOVES = 100;


const int MAX_SEARCH_DEPTH = 60;

struct MoveGenInfo
{
	MoveInfo _availableMoves[MAX_POSSIBLE_MOVES];
	uint16_t _currentMovePos;
	uint16_t _availableMovesSize;
	MoveGenerationStage _nextStage;
	SearchType _searchType;
	MoveInfo _cachedMove;
};

namespace MemPool
{
	// call this function at the beginning of program
	// to initialize memory pool for move generation
	void initMoveGenInfo();

	// deletes all the memory allocated to memory pool
	void destroyMoveGenInfo();


	// Returns the move generator info for the 
	// appropriate depth
	MoveGenInfo* getMoveGenInfo(uint16_t depth);
}

}

#endif
