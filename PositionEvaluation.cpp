#include "PositionEvaluation.h"
#include "PositionState.h"
#include "MoveGenerator.h"

namespace pismo
{

void PositionEvaluation::eval_material(const PositionState& pos)
{
	_pos_value += PAWN_VALUE * (pos.get_white_pieces_count[PAWN_WHITE] - pos.get_black_pieces_count[PAWN_BLACK % (PIECE_NB / 2)]);
	_pos_value += KNIGHT_VALUE * (pos.get_white_pieces_count[KNIGHT_WHITE] - pos.get_black_pieces_count[KNIGHT_BLACK % (PIECE_NB / 2)]);
	_pos_value += BISHOP_VALUE * (pos.get_white_pieces_count[BISHOP_WHITE] - pos.get_black_pieces_count[BISHOP_BLACK % (PIECE_NB / 2)]);
	_pos_value += ROOK_VALUE * (pos.get_white_pieces_count[ROOK_WHITE] - pos.get_black_pieces_count[ROOK_BLACK % (PIECE_NB / 2)]);
	_pos_value += QUEEN_VALUE * (pos.get_white_pieces_count[QUEEN_WHITE] - pos.get_black_pieces_count[QUEEN_BLACK % (PIECE_NB / 2)]);
	_pos_value += KING_VALUE * (pos.get_white_pieces_count[KING_WHITE] - pos.get_black_pieces_count[KING_BLACK % (PIECE_NB / 2)]);
}

void PositionEvaluation::eval_mobility(const PositionState& pos)
{
	unsigned int current_moves_count;
	const MoveGenerator* move_gen = MoveGenerator::instance();
	if (pos.white_to_play()) {
		current_moves_count = move_gen.generate_white_moves(pos).size();
		_pos_value += LEGAL_MOVE_VALUE * (current_moves_count - _prev_moves_count);
	}
	else {
		current_moves_count = move_gen.generate_black_moves(pos).size();
		_pos_value += LEGAL_MOVE_VALUE * (_prev_moves_count - current_moves_count);
	}

	_prev_moves_count = current_moves_count;
}

}







