#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include "utils.h"

namespace pismo
{

//reusing allocated memory to avoid wasting time on system calls (free(), alloc())


//maximum number of moves one side can have in current position
const int MAX_POSSIBLE_MOVES = 100;


const int MAX_SEARCH_DEPTH = 60;
const int MAX_QUIESCENCE_SEARCH_DEPTH = 10;

struct MoveGenInfo
{
	MoveInfo _availableMoves[MAX_POSSIBLE_MOVES];
	uint16_t _currentMovePos;
	uint16_t _badCaptureSize;
	uint16_t _availableMovesSize;
	MoveGenerationStage _nextStage;
	SearchType _searchType;
	MoveInfo _cachedMove;
};

struct CheckPinInfo
{
	Bitboard _directCheck[PIECE_COUNT];
	Bitboard _discPiecePos;
	Bitboard _pinPiecePos;
};

namespace MemPool
{
	// call this function at the beginning of program
	// to initialize memory pool for move generation
	void initMoveGenInfo();

	// deletes all the memory allocated to memory pool
	// for move generation
	void destroyMoveGenInfo();

	// call this function at the beginning of the program
	// to initialize memory pool for position state
	// check and pin info
	void initCheckPinInfo();

	// deletes all the memory allocated to memory pool
	// for position state check and pin info
	void destroyCheckPinInfo();


	// Returns the move generator info for the
	// appropriate depth
	MoveGenInfo* getMoveGenInfo(uint16_t depth);

	// Returns the quiescence search move generator info for the
	// appropriate depth
	MoveGenInfo* getQuiescenceMoveGenInfo(uint16_t depth);

	// Returns the position state check and pin ifno
	// for appropriate depth
	CheckPinInfo* getCheckPinInfo(uint16_t depth);

	// Returns the quiescence search position state check and
	// pin info for appropriate depth
	CheckPinInfo* getQuiescenceCheckPinInfo(uint16_t depth);
}

}

#endif // _MEM_POOL_H_
