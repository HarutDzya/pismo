#include "Core.h"
#include "MoveGenerator.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"
#include "utils.h"
#include <vector>

namespace pismo
{

move_info Core::think(PositionState& pos, uint16_t depth, bool white_to_play)
{
	const std::vector<move_info>& possibleMoves = white_to_play ? 
		MoveGenerator::instance()->generate_white_moves(pos) : MoveGenerator::instance()->generate_black_moves(pos); 

	if (possibleMoves.empty()) {
		return MATE_MOVE; 
	}

	move_info move = possibleMoves[0];
	int16_t score = -MAX_SCORE;
	if (possibleMoves.size() == 1) {
		return move;
	}

	if (white_to_play) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !white_to_play);
			pos.undo_move();
			if (s > score) {
				s = score;
				move = possibleMoves[i];
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !white_to_play);
			pos.undo_move();
			if (s < score) {
				s = score;
				move = possibleMoves[i];
			}
		}
	}
	
	eval_info eval(score, pos.get_zob_key(), depth);
	_trans_table->push(eval);

	return move;
}

int16_t Core::minimax(PositionState& pos, uint16_t depth, bool white_to_play)
{
	eval_info eval;
	if(_trans_table->contains(pos, eval) && eval.depth >= depth) {
		return eval.pos_value;
	}
	
	if (depth == 0) {
		int16_t val = _pos_eval->evaluate(pos);
		eval.pos_value = val;
		eval.depth = 0;
		eval.zob_key = pos.get_zob_key();
		_trans_table->force_push(eval);
		return val;
	}

	const std::vector<move_info>& possibleMoves = white_to_play ?
		MoveGenerator::instance()->generate_white_moves(pos) : MoveGenerator::instance()->generate_black_moves(pos); 
	if (possibleMoves.empty()) {
		return white_to_play ? -MAX_SCORE : MAX_SCORE;
	}
	
	if (white_to_play) {
		int16_t score = -MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !white_to_play);
			pos.undo_move();
			if (s > score) {
			       score = s;
			}
		}

		eval_info eval(score, pos.get_zob_key(), depth);
		_trans_table->force_push(eval);

		return score;
	}
	else {
		int16_t score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !white_to_play);
			pos.undo_move();
			if (s < score) {
			       score = s;
			}
		}

		eval_info eval(score, pos.get_zob_key(), depth);
		_trans_table->force_push(eval);

		return score;
	}
}

Core::Core() : 
_pos_eval(new PositionEvaluation),
_trans_table(new TranspositionTable)
{
}

Core::~Core()
{
	delete _pos_eval;
	delete _trans_table;
}

}
