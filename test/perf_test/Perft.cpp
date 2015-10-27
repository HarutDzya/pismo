#include "Perft.h"
#include "PositionState.h"
#include "MoveGenerator.h"
#include "MemPool.h"

namespace pismo
{

uint64_t Perft::analyze(PositionState& pos, uint16_t depth) const
{
	if (depth == 0) {
		return 1;
	}

	MovesArray& possibleMoves = MemPool::instance()->getMovesArray(depth);
	possibleMoves.clear();

	pos.updateStatePinInfo();
	pos.updateSquaresUnderAttack();
	if (pos.whiteToPlay()) {
		MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves);
	}
	else {
		MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves);
	}

	if (depth == 1) {
		return possibleMoves.size();
	}

	uint64_t moveCount = 0;
	for (unsigned int i = 0; i < possibleMoves.size(); ++i) {
		pos.updateDirectCheckArray();
		pos.updateDiscoveredChecksInfo();
		pos.makeMove(possibleMoves[i]);
		moveCount += analyze(pos, depth - 1);
		pos.undoMove();
	}
	
	return moveCount;
}

Perft::Perft()
{
}

Perft::~Perft()
{
}

}
