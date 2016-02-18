#ifndef ABCORE_H_
#define ABCORE_H_

#include "utils.h"

namespace pismo
{
class PositionState;
class MoveGenerator;
class PositionEvaluation;
class TranspositionTable;

const uint16_t ASP_WINDOW = 40;
const uint16_t DELTA = 20;
const uint16_t ASP_DEPTH = 5;
const uint16_t MAX_TRY = 5;

const uint16_t MAX_QUIESCENCE_DEPTH = 10;

class ABCore
{
public:
	/** Returns the best move in current position.
	 * pos - current position
	 * depth - search depth
	 */

	MoveInfo think(PositionState& pos, uint16_t depth);

	ABCore();
	~ABCore();

private:
	int16_t alphaBetaIterative(uint16_t depth, int16_t alpha, int16_t beta);
	int16_t alphaBeta(uint16_t depth, int16_t alpha, int16_t beta);
	int16_t quiescenceSearch(int16_t qsDepth, int16_t alpha, int16_t beta);
	

	ABCore(const ABCore&); // non-copyable
	ABCore& operator=(const ABCore&); //non-assignable

	PositionState* _pos;
	MoveGenerator* _moveGen;
	PositionEvaluation* _posEval;
	TranspositionTable* _transTable;

};

}

#endif // ABCORE_H_
