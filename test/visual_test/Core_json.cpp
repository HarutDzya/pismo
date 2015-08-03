#include "Core.h"
#include "MoveGenerator.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"
#include "utils.h"
#include <vector>
#include <sstream>

namespace pismo
{

move_info Core::think(PositionState& pos, uint16_t depth, bool white_to_play)
{
file << "{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.get_state_FEN();
file << "\"";

	std::vector<move_info> possibleMoves; 
	white_to_play ? MoveGenerator::instance()->generate_white_moves(pos, possibleMoves) :
	       	MoveGenerator::instance()->generate_black_moves(pos, possibleMoves); 

	if (possibleMoves.empty()) {
    file << ",\n}\n";
		return MATE_MOVE; 
	}

	move_info move = possibleMoves[0];
	int16_t score = -MAX_SCORE;
	if (possibleMoves.size() == 1) {
    file << ",\n}\n";
		return move;
	}

file << ",\n\t\"children\": [";

	if (white_to_play) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);

      
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.get_state_FEN();
file << "\",";

			int16_t s = minimax(pos, depth - 1, !white_to_play);

std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";

			pos.undo_move();
			if (s > score) {
        score = s;
				move = possibleMoves[i];
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);

file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.get_state_FEN();
file << "\",";

			int16_t s = minimax(pos, depth - 1, !white_to_play);

std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";

			pos.undo_move();
			if (s < score) {
        score = s;
				move = possibleMoves[i];
			}
		}
	}

file << "\n\t],";
std::stringstream c;
c << score;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n}\n";

file.close();
	
	//eval_info eval(score, pos.get_zob_key(), depth);
	//_trans_table->push(eval);
  

	return move;
}

int16_t Core::minimax(PositionState& pos, uint16_t depth, bool white_to_play)
{
	eval_info eval;
//	if(_trans_table->contains(pos, eval) && eval.depth >= depth) {
//		return eval.pos_value;
//	}
	
	if (depth == 0) {
    leaf++;
		int16_t val = _pos_eval->evaluate(pos);
		eval.pos_value = val;
		eval.depth = 0;
		eval.zob_key = pos.get_zob_key();
		_trans_table->force_push(eval);
		return val;
	}

if (depth > 2) {
file << "\n\t\"children\": [";
}

	std::vector<move_info> possibleMoves;
	white_to_play ? MoveGenerator::instance()->generate_white_moves(pos, possibleMoves) :
	       MoveGenerator::instance()->generate_black_moves(pos, possibleMoves); 
	if (possibleMoves.empty()) {
    leaf++;
		return white_to_play ? -MAX_SCORE : MAX_SCORE;
	}
	
	int16_t score = -MAX_SCORE;
	if (white_to_play) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {


			pos.make_move(possibleMoves[i]);
if (depth > 2) {
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.get_state_FEN();
file << "\",";
}


			int16_t s = minimax(pos, depth - 1, !white_to_play);

if (depth > 2) {
std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";
}

			pos.undo_move();
			if (s > score) {
			       score = s;
			}
		}

		//eval_info eval(score, pos.get_zob_key(), depth);
		//_trans_table->force_push(eval);

	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.make_move(possibleMoves[i]);

if (depth > 2) {
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.get_state_FEN();
file << "\",";
}

			int16_t s = minimax(pos, depth - 1, !white_to_play);

if (depth > 2) {
std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";
}

			pos.undo_move();
			if (s < score) {
			       score = s;
			}
		}

		//eval_info eval(score, pos.get_zob_key(), depth);
		//_trans_table->force_push(eval);

	}
if (depth > 2) {
  file << "\n\t],";
}

  return score;

}

Core::Core() : 
_pos_eval(new PositionEvaluation),
_trans_table(new TranspositionTable)
{
  leaf = 0;
  file.open("my.json");
}

Core::~Core()
{
	delete _pos_eval;
	delete _trans_table;
  file.close();
}

}
