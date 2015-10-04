#include "Core.h"
#include "MoveGenerator.h"
#include "MemPool.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"

namespace pismo
{

MoveInfo Core::think(PositionState& pos, uint16_t depth)
{
	MovesArray& possibleMoves = MemPool::instance()->getMovesArray(depth);
	possibleMoves.clear();	
	pos.whiteToPlay() ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
	       	MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves); 

	if (possibleMoves.empty()) {
		return MATE_MOVE; 
	}

	MoveInfo move = possibleMoves[0];
	int16_t score = -MAX_SCORE;
	if (possibleMoves.size() == 1) {
		return move;
	}

	if (pos.whiteToPlay()) {
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, pos.whiteToPlay());
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
			int16_t s = minimax(pos, depth - 1, pos.whiteToPlay());
			pos.undoMove();
			if (s < score) {
				score = s;
				move = possibleMoves[i];
			}
		}
	}
	
	EvalInfo eval(score, pos.getZobKey(), depth);
	_transTable->push(eval);

	return move;
}

int16_t Core::minimax(PositionState& pos, uint16_t depth, bool whiteToPlay)
{
	EvalInfo eval;
	if(_transTable->contains(pos, eval) && eval.depth >= depth) {
		return eval.posValue;
	}
	
	if (depth == 0) {
		int16_t val = _posEval->evaluate(pos);
		eval.posValue = val;
		eval.depth = 0;
		eval.zobKey = pos.getZobKey();
		_transTable->forcePush(eval);
		return val;
	}

	MovesArray& possibleMoves = MemPool::instance()->getMovesArray(depth);
	possibleMoves.clear();
	whiteToPlay ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
	       MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves); 
	if (possibleMoves.empty()) {
		return whiteToPlay ? -MAX_SCORE : MAX_SCORE;
	}
	
	if (whiteToPlay) {
		int16_t score = -MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !whiteToPlay);
			pos.undoMove();
			if (s > score) {
			       score = s;
			}
		}

		EvalInfo eval(score, pos.getZobKey(), depth);
		_transTable->forcePush(eval);

		return score;
	}
	else {
		int16_t score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = minimax(pos, depth - 1, !whiteToPlay);
			pos.undoMove();
			if (s < score) {
			       score = s;
			}
		}

		EvalInfo eval(score, pos.getZobKey(), depth);
		_transTable->forcePush(eval);

		return score;
	}
}

Core::Core() : 
_posEval(new PositionEvaluation),
_transTable(new TranspositionTable)
{
}

Core::~Core()
{
	delete _posEval;
	delete _transTable;
}

}
