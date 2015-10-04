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

MoveInfo Core::think(PositionState& pos, uint16_t depth, bool whiteToPlay)
{
file << "{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.getStateFEN();
file << "\"";

	std::vector<MoveInfo> possibleMoves; 
	whiteToPlay ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
	       	MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves); 

	if (possibleMoves.empty()) {
    file << ",\n}\n";
		return MATE_MOVE; 
	}

	MoveInfo move = possibleMoves[0];
	int16_t score = -MAX_SCORE;
	if (possibleMoves.size() == 1) {
    file << ",\n}\n";
		return move;
	}

file << ",\n\t\"children\": [";

	if (whiteToPlay) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);

      
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.getStateFEN();
file << "\",";

			int16_t s = minimax(pos, depth - 1, !whiteToPlay);

std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";

			pos.undoMove();
			if (s > score) {
        score = s;
				move = possibleMoves[i];
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);

file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.getStateFEN();
file << "\",";

			int16_t s = minimax(pos, depth - 1, !whiteToPlay);

std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";

			pos.undoMove();
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
	
	//EvalInfo eval(score, pos.getZobKey(), depth);
	//_transTable->push(eval);
  

	return move;
}

int16_t Core::minimax(PositionState& pos, uint16_t depth, bool whiteToPlay)
{
	evalInfo eval;
//	if(_transTable->contains(pos, eval) && eval.depth >= depth) {
//		return eval.posValue;
//	}
	
	if (depth == 0) {
    leaf++;
		int16_t val = _posEval->evaluate(pos);
		eval.posValue = val;
		eval.depth = 0;
		eval.zobKey = pos.getZobKey();
		_transTable->forcePush(eval);
		return val;
	}

if (depth > 2) {
file << "\n\t\"children\": [";
}

	std::vector<MoveInfo> possibleMoves;
	whiteToPlay ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
	       MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves); 
	if (possibleMoves.empty()) {
    leaf++;
		return whiteToPlay ? -MAX_SCORE : MAX_SCORE;
	}
	
	int16_t score = -MAX_SCORE;
	if (whiteToPlay) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {


			pos.makeMove(possibleMoves[i]);
if (depth > 2) {
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.getStateFEN();
file << "\",";
}


			int16_t s = minimax(pos, depth - 1, !whiteToPlay);

if (depth > 2) {
std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";
}

			pos.undoMove();
			if (s > score) {
			       score = s;
			}
		}

		//EvalInfo eval(score, pos.getZobKey(), depth);
		//_transTable->forcePush(eval);

	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);

if (depth > 2) {
file << "\n\t{\n\t\"name\":\"o\",\n\t\"fen\": \"";
file << pos.getStateFEN();
file << "\",";
}

			int16_t s = minimax(pos, depth - 1, !whiteToPlay);

if (depth > 2) {
std::stringstream c;
c << s;
file << "\n\t\"score\":\"" << c.str() << "\"";
file << "\n\t}";
if (i != possibleMoves.size() - 1) file << ",";
}

			pos.undoMove();
			if (s < score) {
			       score = s;
			}
		}

		//EvalInfo eval(score, pos.getZobKey(), depth);
		//_transTable->forcePush(eval);

	}
if (depth > 2) {
  file << "\n\t],";
}

  return score;

}

Core::Core() : 
_posEval(new PositionEvaluation),
_transTable(new TranspositionTable)
{
  leaf = 0;
  file.open("my.json");
}

Core::~Core()
{
	delete _posEval;
	delete _transTable;
  file.close();
}

}
