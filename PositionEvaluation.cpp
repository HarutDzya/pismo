#include "PositionEvaluation.h"
#include "PositionState.h"

namespace pismo
{
PositionEvaluation::PositionEvaluation():
	_pos_value(0),
	_material_hash(MATERIAL_HASH_SIZE)
{
}


int16_t PositionEvaluation::evaluate(const PositionState& pos)
{
	_pos_value = 0;
	eval_material(pos);
	eval_piece_square(pos);
	eval_mobility(pos);
	// king_safety();
	
	return _pos_value;
}

void PositionEvaluation::eval_material(const PositionState& pos)
{
	int16_t material_value = 0;
	unsigned int index = hash_function(pos.get_material_zob_key());
	if (_material_hash[index].material_zob_key == pos.get_material_zob_key()) {
		material_value = _material_hash[index].material_value;
	}
	else {	
		for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
			material_value += pos.get_piece_count((Piece) piece) * PIECE_VALUES[piece];
		}
		_material_hash[index] = material_info(material_value, pos.get_material_zob_key());
	}
	
	_pos_value += material_value;
}

void PositionEvaluation::eval_piece_square(const PositionState& pos)
{
	_pos_value += pos.get_pst_value();
}

void PositionEvaluation::eval_mobility(const PositionState& pos)
{
}

unsigned int PositionEvaluation::hash_function(const ZobKey& material_zob_key) const
{
	return (material_zob_key % MATERIAL_HASH_SIZE);
       // TODO: Change the impl to return first log(MATERIAL_HASH_SIZE) + 1 bits of material_zob_key	

}

}







