#include "PositionEvaluation.h"
#include "PositionState.h"

namespace pismo
{

void PositionEvaluation::eval_material(const PositionState& pos)
{
	int material_value;
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
		material_value += pos.get_piece_count(piece) * PIECE_VALUES[piece];
	}
	if (material_value == pos.get_material_value()) {
		_pos_value = (pos.get_material_value() + pos.get_pst_value()) / 100.00;
	}
}

void PositionEvaluation::eval_mobility(const PositionState& pos)
{
}

}







