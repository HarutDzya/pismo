#include "ABCore.h"
#include "MoveGenerator.h"
#include "MemPool.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"

namespace pismo
{
move_info ABCore::think(PositionState& pos, uint16_t depth)
{
	moves_array& possible_moves = MemPool::instance()->get_moves_array(depth);
	possible_moves.clear();
	pos.white_to_play() ? MoveGenerator::instance()->generate_white_moves(pos, possible_moves) :
		MoveGenerator::instance()->generate_black_moves(pos, possible_moves);

	if (possible_moves.empty()) {
		return MATE_MOVE;
	}

	if (possible_moves.size() == 1) {
		return possible_moves[0];
	}

	move_info move;
	int16_t score;
	if (pos.white_to_play()) {
		score = -MAX_SCORE;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			int16_t s = alpha_beta_iterative(pos, depth - 1, score, MAX_SCORE, pos.white_to_play());
			pos.undo_move();
			if (s > score) {
				score = s;
				move = possible_moves[i];
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			int16_t s = alpha_beta_iterative(pos, depth - 1, -MAX_SCORE, score, pos.white_to_play());
			pos.undo_move();
			if (s < score) {
				score = s;
				move = possible_moves[i];
			}
		}
	}
	
	eval_info eval(score, pos.get_zob_key(), depth);
	_trans_table->push(eval);

	return move;
}

int16_t ABCore::alpha_beta_iterative(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool white_to_play)
{
	int16_t score;
	uint16_t current_depth = 0;
	int16_t current_alpha = alpha;
	int16_t current_beta = beta;
	uint16_t try_count = 0;
	while (current_depth <= depth) {
		score = alpha_beta(pos, current_depth, current_alpha, current_beta, white_to_play);
		if (score <= current_alpha) {
			if (try_count < MAX_TRY) {
				current_alpha -= (try_count + 1) * DELTA;
				++try_count;
			}
			else {
				current_alpha = -MAX_SCORE;
			}
		}
		else if (score >= current_beta) {
			if (try_count < MAX_TRY) {
				current_beta += (try_count + 1) * DELTA;
				++try_count;
			}
			else {
				current_beta = MAX_SCORE;
			}
		}
		else {
			if (current_depth >= ASP_DEPTH) { 
				current_alpha = score - ASP_WINDOW;
				current_beta = score + ASP_WINDOW;
			}
			try_count = 0;
			++current_depth;
		}
	}
	
	return score;
}

int16_t ABCore::alpha_beta(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool white_to_play)
{
	eval_info eval;
	if (_trans_table->contains(pos, eval) && eval.depth >= depth) {
		return eval.pos_value;
	}

	if (depth == 0) {
		return quiescence_search(pos, depth, alpha, beta, white_to_play);
	}

	moves_array& possible_moves = MemPool::instance()->get_moves_array(depth);
	possible_moves.clear();
	white_to_play ? MoveGenerator::instance()->generate_white_moves(pos, possible_moves) :
		MoveGenerator::instance()->generate_black_moves(pos, possible_moves);

	int16_t score;
	int16_t current_alpha = alpha;
	int16_t current_beta = beta;
	if (white_to_play) {
		score = -MAX_SCORE;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			int16_t s = alpha_beta(pos, depth - 1, current_alpha, current_beta, !white_to_play);
			pos.undo_move();
			if (s > score) {
				score = s;
			}
			if (score > current_alpha) {
				current_alpha = score;
			}
			if (score >= current_beta) {
				break;
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			int16_t s = alpha_beta(pos, depth - 1, current_alpha, current_beta, !white_to_play);
			pos.undo_move();
			if (s < score) {
				score = s;
			}
			if (score < current_beta) {
				current_beta = score;
			}
			if (score <= current_alpha) {
				break;
			}
		}
	}
	
	if (score > alpha && score < beta) {
		eval.pos_value = score;
		eval.zob_key = pos.get_zob_key();
		eval.depth = depth;
		_trans_table->force_push(eval);
	}

	return score;
}

int16_t ABCore::quiescence_search(PositionState& pos, int16_t qs_depth, int16_t alpha, int16_t beta, bool white_to_play)
{
	//TODO : Consider check evasions
	eval_info eval;
	int16_t val;
	if (_trans_table->contains(pos, eval) && eval.depth >= 0) {
		val = eval.pos_value;
	}
	else {
		val = _pos_eval->evaluate(pos);
		eval.pos_value = val;
		eval.zob_key = pos.get_zob_key();
		eval.depth = 0;
		_trans_table->force_push(eval);
	}

	if (qs_depth == -MAX_QUIET_DEPTH) {
		return val;
	}

	int16_t current_alpha = alpha;
	int16_t current_beta = beta;
	if (white_to_play) {	
		if (val >= current_beta) {
			return val;
		}
	
		if (val > current_alpha) {
			current_alpha = val;
		}
		moves_array possible_moves; //TODO: Implement memory pool for quiescence search
		MoveGenerator::instance()->generate_white_non_quiet_moves(pos, possible_moves);
		int16_t score;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			score = quiescence_search(pos, qs_depth - 1, current_alpha, current_beta, !white_to_play);
			pos.undo_move();
			if (score > current_alpha) {
				current_alpha = score;
			}
			if (score >= current_beta) {
				break;
			} 
		}
		return current_alpha;	
	}
	else {
		if (val <= current_alpha) {
			return val;
		}
	
		if (val < current_beta) {
			current_beta = val;
		}
		moves_array possible_moves; //TODO: Implement memory pool for quiescence search
		MoveGenerator::instance()->generate_black_non_quiet_moves(pos, possible_moves);
		int16_t score;
		for (uint16_t i = 0; i < possible_moves.size(); ++i) {
			pos.make_move(possible_moves[i]);
			score = quiescence_search(pos, qs_depth - 1, current_alpha, current_beta, !white_to_play);
			pos.undo_move();
			if (score < current_beta) {
				current_beta = score;
			}
			if (score <= current_alpha) {
				break;
			} 
		}
		return current_beta;	
	}
}

ABCore::ABCore() :
_pos_eval(new PositionEvaluation()),
_trans_table(new TranspositionTable())
{
}

ABCore::~ABCore()
{
	delete _pos_eval;
	delete _trans_table;
}

} 	
