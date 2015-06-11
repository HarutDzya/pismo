#include "MoveGenerator.h"
#include "PositionState.h"

namespace pismo
{
std::vector<move_info> MoveGenerator::generate_white_moves(const PositionState& pos)
{
	return pos.get_generated_moves(WHITE);
}

std::vector<move_info> MoveGenerator::generate_black_moves(const PositionState& pos)
{
	return pos.get_generated_moves(BLACK);
}

}
