#include "Perft.h"
#include <vector>
#include "PositionState.h"
#include "MoveGenerator.h"

namespace pismo
{

uint64_t Perft::analyze(PositionState& pos, uint16_t depth) const
{
	if (depth == 0) {
		return 1;
	}

	std::vector<move_info> possibleMoves;
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
		count_moves(pos, depth - 1, move_count);
		pos.undo_move();
	}
	
	return move_count;
}

void Perft::count_moves(PositionState& pos, uint16_t depth, uint64_t& move_count) const
{
	std::vector<move_info> possibleMoves;
	if (pos.white_to_play()) {
		MoveGenerator::instance()->generate_white_moves(pos, possibleMoves);
	}
	else {
		MoveGenerator::instance()->generate_black_moves(pos, possibleMoves);
	}

	if (depth == 1) {
		move_count += possibleMoves.size();
	}
	else {
		for (unsigned int i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);
			count_moves(pos, depth - 1, move_count);
			pos.undo_move();
		}
	}
}

Perft::Perft()
{
}

Perft::~Perft()
{
}

}
