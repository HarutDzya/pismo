#ifndef ABCORE_H_
#define ABCORE_H_

#include "utils.h"

namespace pismo
{
class PositionState;
class PositionEvaluation;
class TranspositionTable;

const uint16_t ASP_WINDOW = 40;
const uint16_t DELTA = 20;
const uint16_t ASP_DEPTH = 5;
const uint16_t MAX_TRY = 5;

const uint16_t MAX_QUIET_DEPTH = 10; 

class ABCore
{
public:
	/** Returns the best move in current position.
	 * pos - current position
	 * depth - search depth
	 */

	move_info think(PositionState& pos, uint16_t depth);

	ABCore();
	~ABCore();

private:
	int16_t alpha_beta_iterative(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool white_to_play);
	int16_t alpha_beta(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool white_to_play);
	int16_t quiescence_search(PositionState& pos, int16_t qs_depth, int16_t alpha, int16_t beta, bool white_to_play);
	

	ABCore(const ABCore&); // non-copyable
	ABCore& operator=(const ABCore&); //non-assignable

	PositionEvaluation* _pos_eval;
	TranspositionTable* _trans_table;

};

}

#endif // ABCORE_H_
