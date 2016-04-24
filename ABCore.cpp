#include "ABCore.h"
#include "MoveGenerator.h"
#include "PositionState.h"
#include "PositionEvaluation.h"
#include "TranspositionTable.h"

namespace pismo
{
namespace UCI
{
extern bool stopSearch;
}

MoveInfo ABCore::think(PositionState& pos, uint16_t depth)
{
	_pos = &pos;
	_moveGen = MoveGenerator::instance();

	_moveGen->prepareMoveGeneration(_pos->kingUnderCheck() ? EVASION_SEARCH : USUAL_SEARCH,
			MoveInfo(), depth);
	_pos->initCheckPinInfo(depth);

	MoveInfo generatedMove = _moveGen->getTopMove(*_pos, depth);
	MoveInfo move = MATE_MOVE;
	int16_t score;
	if (_pos->whiteToPlay()) {
		score = -MAX_SCORE;
		while(generatedMove.from != INVALID_SQUARE && !UCI::stopSearch) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				if (move.from == INVALID_SQUARE) {
					move = generatedMove;
				}
				_pos->makeMove(generatedMove);
				int16_t s = alphaBetaIterative(depth - 1, score, MAX_SCORE);
				_pos->undoMove();
				if (s > score) {
					score = s;
					move = generatedMove;
				}
				_pos->updateCheckPinInfo(depth);
			}
			generatedMove = _moveGen->getTopMove(*_pos, depth);
		}
	}
	else {
		score = MAX_SCORE;
		while(generatedMove.from != INVALID_SQUARE && !UCI::stopSearch) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				if (move.from == INVALID_SQUARE) {
					move = generatedMove;
				}
				_pos->makeMove(generatedMove);
				int16_t s = alphaBetaIterative(depth - 1, -MAX_SCORE, score);
				_pos->undoMove();
				if (s < score) {
					score = s;
					move = generatedMove;
				}
				_pos->updateCheckPinInfo(depth);
			}
			generatedMove = _moveGen->getTopMove(*_pos, depth);
		}
	}
	
	EvalInfo eval(score, pos.getZobKey(), depth);
	_transTable->push(eval);

	return move;
}

int16_t ABCore::alphaBetaIterative(uint16_t depth, int16_t alpha, int16_t beta)
{
	int16_t score;
	uint16_t currentDepth = 0;
	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	uint16_t tryCount = 0;
	while (currentDepth <= depth) {
		score = alphaBeta(currentDepth, currentAlpha, currentBeta);
		if (score == MAX_SCORE || score == -MAX_SCORE) {
			return score;
		}
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

int16_t ABCore::alphaBeta(uint16_t depth, int16_t alpha, int16_t beta)
{
	if (depth == 0) {
		return quiescenceSearch(depth, alpha, beta);
	}

	EvalInfo eval;
	if (_transTable->contains(*_pos, eval) && eval.depth >= depth) {
		return eval.posValue;
	}

	_moveGen->prepareMoveGeneration(_pos->kingUnderCheck() ? EVASION_SEARCH : USUAL_SEARCH,
			MoveInfo(), depth);
	_pos->initCheckPinInfo(depth);

	MoveInfo generatedMove = _moveGen->getTopMove(*_pos, depth);

	int16_t score;
	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	if (_pos->whiteToPlay()) {
		score = -MAX_SCORE;
		while(generatedMove.from != INVALID_SQUARE) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				_pos->makeMove(generatedMove);
				int16_t s = alphaBeta(depth - 1, currentAlpha, currentBeta);
				_pos->undoMove();
				if (s > score) {
					score = s;
					if (score > currentAlpha) {
						currentAlpha = score;
					}
					if (score >= currentBeta) {
						break;
					}
				}
				_pos->updateCheckPinInfo(depth);
			}
			generatedMove = _moveGen->getTopMove(*_pos, depth);
		}
	}
	else {
		score = MAX_SCORE;
		while(generatedMove.from != INVALID_SQUARE) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				_pos->makeMove(generatedMove);
				int16_t s = alphaBeta(depth - 1, currentAlpha, currentBeta);
				_pos->undoMove();
				if (s < score) {
					score = s;
					if (score < currentBeta) {
						currentBeta = score;
					}
					if (score <= currentAlpha) {
						break;
					}
				}
				_pos->updateCheckPinInfo(depth);
			}
			generatedMove = _moveGen->getTopMove(*_pos, depth);
		}
	}
	
	if (score > alpha && score < beta) {
		eval.posValue = score;
		eval.zobKey = _pos->getZobKey();
		eval.depth = depth;
		_transTable->forcePush(eval);
	}

	return score;
}

int16_t ABCore::quiescenceSearch(int16_t qsDepth, int16_t alpha, int16_t beta)
{
	EvalInfo eval;
	int16_t val;
	if (_transTable->contains(*_pos, eval)) {
	   if (eval.depth > 0) {
		   return eval.posValue;
	   }
	   else {
		   val = eval.posValue;
	   }
	}
	else {
		val = _posEval->evaluate(*_pos);
		eval.posValue = val;
		eval.zobKey = _pos->getZobKey();
		eval.depth = 0;
		_transTable->forcePush(eval);
	}

	if (qsDepth == MAX_QUIESCENCE_DEPTH) {
		return val;
	}

	int16_t currentAlpha = alpha;
	int16_t currentBeta = beta;
	int16_t score;
	if (_pos->whiteToPlay()) {
		if (val >= currentBeta) {
			return val;
		}
	
		if (val > currentAlpha) {
			currentAlpha = val;
		}
		_moveGen->prepareMoveGeneration(QUIESCENCE_SEARCH, MoveInfo(), qsDepth);
		_pos->initCheckPinInfo(qsDepth, true);

		MoveInfo generatedMove = _moveGen->getTopMove(*_pos, qsDepth, true);
		while(generatedMove.from != INVALID_SQUARE) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				_pos->makeMove(generatedMove);
				score = quiescenceSearch(qsDepth + 1, currentAlpha, currentBeta);
				_pos->undoMove();
				if (score > currentAlpha) {
					currentAlpha = score;
				}
				if (score >= currentBeta) {
					break;
				}
				_pos->updateCheckPinInfo(qsDepth, true);
			}
			generatedMove = _moveGen->getTopMove(*_pos, qsDepth, true);
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
		_moveGen->prepareMoveGeneration(QUIESCENCE_SEARCH, MoveInfo(), qsDepth);
		_pos->initCheckPinInfo(qsDepth, true);

		MoveInfo generatedMove = _moveGen->getTopMove(*_pos, qsDepth, true);
		int16_t score;
		while(generatedMove.from != INVALID_SQUARE) {
			if (_pos->pseudoMoveIsLegalMove(generatedMove)) {
				_pos->makeMove(generatedMove);
				score = quiescenceSearch(qsDepth + 1, currentAlpha, currentBeta);
				_pos->undoMove();
				if (score < currentBeta) {
					currentBeta = score;
				}
				if (score <= currentAlpha) {
					break;
				}
				_pos->updateCheckPinInfo(qsDepth, true);
			}
			generatedMove = _moveGen->getTopMove(*_pos, qsDepth, true);
		}

		return currentBeta;
	}
}

ABCore::ABCore() :
_posEval(new PositionEvaluation()),
_transTable(new TranspositionTable())
{
	_posEval->initPosEval();
}

ABCore::~ABCore()
{
	delete _posEval;
	delete _transTable;
}

} 	
