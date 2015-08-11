#include "Perft.h"
#include "PositionState.h"
#include "MoveGenerator.h"
#include "MemPool.h"

namespace pismo
{

uint64_t Perft::analyze(PositionState& pos, uint16_t depth) const
{
	if (depth == 0) {
		return 1;
	}

	moves_array& possibleMoves = MemPool::instance()->get_moves_array(depth);
	possibleMoves.clear();
	if (pos.white_to_play()) {
		MoveGenerator::instance()->generate_white_moves(pos, possibleMoves);
	}
	else {
		MoveGenerator::instance()->generate_black_moves(pos, possibleMoves);
	}

	if (depth == 1) {
		return possibleMoves.size();
	}

	uint64_t move_count = 0;
	for (unsigned int i = 0; i < possibleMoves.size(); ++i) {
		pos.make_move(possibleMoves[i]);
		move_count += analyze(pos, depth - 1);
		pos.undo_move();
	}
	
	return move_count;
}

Perft::Perft()
{
}

Perft::~Perft()
{
}

}
