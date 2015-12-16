#include "Perft.h"
#include "PositionState.h"
#include "MoveGenerator.h"
#include "MemPool.h"
#include <iostream>

namespace pismo
{

uint64_t Perft::analyze(PositionState& pos, uint16_t depth, bool begin) const
{
	if (depth == 0) {
		return 1;
	}

	if (!pos.kingUnderCheck()) {
		MoveGenerator::instance()->prepareMoveGeneration(USUAL_SEARCH, MATE_MOVE, depth);
	}
	else {
		MoveGenerator::instance()->prepareMoveGeneration(EVASION_SEARCH, MATE_MOVE, depth);
	}


	uint64_t moveCount = 0;
	MoveInfo move = MoveGenerator::instance()->getTopMove(pos, depth);
	pos.updateStatePinInfo();
	while (move.from != INVALID_SQUARE && move.to != INVALID_SQUARE) {
		uint64_t mc = 0;
		if (pos.pseudoMoveIsLegalMove(move)) {
			pos.updateDirectCheckArray();
			pos.updateDiscoveredChecksInfo();
			pos.makeMove(move);
			mc = analyze(pos, depth - 1);
			pos.undoMove();
			pos.updateStatePinInfo();
		}
		moveCount += mc;
    	if (begin) {
      		//std::cout << "Move: " << pismo::moveToNotation(move) << "    " << mc << "   FEN: " << pos.getStateFEN() << std::endl;
    	}
		move = MoveGenerator::instance()->getTopMove(pos, depth);
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
