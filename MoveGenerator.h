#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class PositionState;

class MoveGenerator
{
public:
	static std::vector<move_info> generate_white_moves(const PositionState& pos);
	static std::vector<move_info> generate_black_moves(const PositionState& pos);
};

}

#endif
