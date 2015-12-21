#include "Perft.h"
#include "PositionState.h"
#include "MoveGenerator.h"
#include "MemPool.h"
#include <iostream>

namespace pismo
{

uint64_t Perft::analyze(PositionState& pos, uint16_t depth, bool begin) const
{
  if (depth == 0)
    return 1;

  MoveGenerator::instance()->generatePerftMoves(pos, depth);

  uint64_t moveCount = 0;
  MoveGenInfo* genInfo = MemPool::getMoveGenInfo(depth);
  pos.updateStatePinInfo();
  while (genInfo->_currentMovePos < genInfo->_availableMovesSize)
  {
    uint64_t mc = 0;
    if (pos.pseudoMoveIsLegalMove((genInfo->_availableMoves)[genInfo->_currentMovePos++]))
    {
      pos.updateDirectCheckArray();
      pos.updateDiscoveredChecksInfo();
      pos.makeMove((genInfo->_availableMoves)[genInfo->_currentMovePos - 1]);
      mc = analyze(pos, depth - 1);
      pos.undoMove();
      pos.updateStatePinInfo();
    }
    moveCount += mc;
    if (begin) {
		//std::cout << "Move: " << pismo::moveToNotation((genInfo->_availableMoves)[genInfo->_currentMovePos - 1]) << "    " << mc << "   FEN: " << pos.getStateFEN() << std::endl;
    }
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
