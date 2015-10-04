#include "ABCore.h"
#include "MoveGenerator.h"
#include "MemPool.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"

namespace pismo
{
MoveInfo ABCore::think(PositionState& pos, uint16_t depth)
{
	MovesArray& possibleMoves = MemPool::instance()->getMovesArray(depth);
	possibleMoves.clear();
	pos.whiteToPlay() ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
		MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves);

	if (possibleMoves.empty()) {
		return MATE_MOVE;
	}

	if (possibleMoves.size() == 1) {
		return possibleMoves[0];
	}

	MoveInfo move;
	int16_t score;
	if (pos.whiteToPlay()) {
		score = -MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = alphaBetaIterative(pos, depth - 1, score, MAX_SCORE, pos.whiteToPlay());
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
			int16_t s = alphaBetaIterative(pos, depth - 1, -MAX_SCORE, score, pos.whiteToPlay());
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

int16_t ABCore::alphaBetaIterative(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool whiteToPlay)
{
	int16_t score;
	uint16_t currentDepth = 0;
	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	uint16_t tryCount = 0;
	while (currentDepth <= depth) {
		score = alphaBeta(pos, currentDepth, currentAlpha, currentBeta, whiteToPlay);
		if (score <= currentAlpha) {
			if (tryCount < MAX_TRY) {
				currentAlpha -= (tryCount + 1) * DELTA;
				++tryCount;
			}
			else {
				currentAlpha = -MAX_SCORE;
			}
		}
		else if (score >= currentBeta) {
			if (tryCount < MAX_TRY) {
				currentBeta += (tryCount + 1) * DELTA;
				++tryCount;
			}
			else {
				currentBeta = MAX_SCORE;
			}
		}
		else {
			if (currentDepth >= ASP_DEPTH) { 
				currentAlpha = score - ASP_WINDOW;
				currentBeta = score + ASP_WINDOW;
			}
			tryCount = 0;
			++currentDepth;
		}
	}
	
	return score;
}

int16_t ABCore::alphaBeta(PositionState& pos, uint16_t depth, int16_t alpha, int16_t beta, bool whiteToPlay)
{
	if (depth == 0) {
		return quiescenceSearch(pos, depth, alpha, beta, whiteToPlay);
	}

	EvalInfo eval;
	if (_transTable->contains(pos, eval) && eval.depth >= depth) {
		return eval.posValue;
	}

	MovesArray& possibleMoves = MemPool::instance()->getMovesArray(depth);
	possibleMoves.clear();
	whiteToPlay ? MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves) :
		MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves);

	int16_t score;
	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	if (whiteToPlay) {
		score = -MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = alphaBeta(pos, depth - 1, currentAlpha, currentBeta, !whiteToPlay);
			pos.undoMove();
			if (s > score) {
				score = s;
			}
			if (score > currentAlpha) {
				currentAlpha = score;
			}
			if (score >= currentBeta) {
				break;
			}
		}
	}
	else {
		score = MAX_SCORE;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			int16_t s = alphaBeta(pos, depth - 1, currentAlpha, currentBeta, !whiteToPlay);
			pos.undoMove();
			if (s < score) {
				score = s;
			}
			if (score < currentBeta) {
				currentBeta = score;
			}
			if (score <= currentAlpha) {
				break;
			}
		}
	}
	
	if (score > alpha && score < beta) {
		eval.posValue = score;
		eval.zobKey = pos.getZobKey();
		eval.depth = depth;
		_transTable->forcePush(eval);
	}

	return score;
}

int16_t ABCore::quiescenceSearch(PositionState& pos, int16_t qsDepth, int16_t alpha, int16_t beta, bool whiteToPlay)
{
	//TODO : Consider check evasions
	EvalInfo eval;
	int16_t val;
	if (_transTable->contains(pos, eval) && eval.depth >= 0) {
		val = eval.posValue;
	}
	else {
		val = _posEval->evaluate(pos);
		eval.posValue = val;
		eval.zobKey = pos.getZobKey();
		eval.depth = 0;
		_transTable->forcePush(eval);
	}

	if (qsDepth == -MAX_QUIET_DEPTH) {
		return val;
	}

	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	if (whiteToPlay) {	
		if (val >= currentBeta) {
			return val;
		}
	
		if (val > currentAlpha) {
			currentAlpha = val;
		}
		MovesArray possibleMoves; //TODO: Implement memory pool for quiescence search
		//MoveGenerator::instance()->generateWhiteNonQuietMoves(pos, possibleMoves);
		int16_t score;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			score = quiescenceSearch(pos, qsDepth - 1, currentAlpha, currentBeta, !whiteToPlay);
			pos.undoMove();
			if (score > currentAlpha) {
				currentAlpha = score;
			}
			if (score >= currentBeta) {
				break;
			} 
		}
		return currentAlpha;	
	}
	else {
		if (val <= currentAlpha) {
			return val;
		}
	
		if (val < currentBeta) {
			currentBeta = val;
		}
		MovesArray possibleMoves; //TODO: Implement memory pool for quiescence search
		//MoveGenerator::instance()->generateBlackNonQuietMoves(pos, possibleMoves);
		int16_t score;
		for (uint16_t i = 0; i < possibleMoves.size(); ++i) {
			pos.makeMove(possibleMoves[i]);
			score = quiescenceSearch(pos, qsDepth - 1, currentAlpha, currentBeta, !whiteToPlay);
			pos.undoMove();
			if (score < currentBeta) {
				currentBeta = score;
			}
			if (score <= currentAlpha) {
				break;
			} 
		}
		return currentBeta;	
	}
}

ABCore::ABCore() :
_posEval(new PositionEvaluation()),
_transTable(new TranspositionTable())
{
}

ABCore::~ABCore()
{
	delete _posEval;
	delete _transTable;
}

} 	
