#ifndef _POSSIBLE_MOVES_H_
#define _POSSIBLE_MOVES_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class PossibleMoves
{
public:
	PossibleMoves();

	const std::vector<Square>& possibleWhitePawnMoves(Square from) const;
	const std::vector<Square>& possibleBlackPawnMoves(Square from) const;
	const std::vector<Square>& possibleKnightMoves(Square from) const;
	const std::vector<Square>& possibleKingMoves(Square from) const;
	const std::vector<Square>& possibleLeftRankMoves(Square from) const;
	const std::vector<Square>& possibleRightRankMoves(Square from) const;
	const std::vector<Square>& possibleUpFileMoves(Square from) const;
	const std::vector<Square>& possibleDownFileMoves(Square from) const;
	const std::vector<Square>& possibleUpDiagA1h8Moves(Square from) const;
	const std::vector<Square>& possibleDownDiagA1h8Moves(Square from) const;
	const std::vector<Square>& possibleUpDiagA8h1Moves(Square from) const;
	const std::vector<Square>& possibleDownDiagA8h1Moves(Square from) const;


private:
	void initWhitePawnMoves();
	void initBlackPawnMoves();
	void initKnightMoves();
	void initKingMoves();
	void initLeftRankMoves();
	void initRightRankMoves();
	void initUpFileMoves();
	void initDownFileMoves();
	void initUpDiagA1h8Moves();
	void initDownDiagA1h8Moves();
	void initUpDiagA8h1Moves();
	void initDownDiagA8h1Moves();

private:
	std::vector<std::vector<Square> > _whitePawnMovesList;
	std::vector<std::vector<Square> > _blackPawnMovesList;
	std::vector<std::vector<Square> > _knightMovesList;
	std::vector<std::vector<Square> > _kingMovesList;
	std::vector<std::vector<Square> > _leftRankMovesList;
	std::vector<std::vector<Square> > _rightRankMovesList;
	std::vector<std::vector<Square> > _upFileMovesList;
	std::vector<std::vector<Square> > _downFileMovesList;
	std::vector<std::vector<Square> > _upDiagA1h8MovesList;
	std::vector<std::vector<Square> > _downDiagA1h8MovesList;
	std::vector<std::vector<Square> > _upDiagA8h1MovesList;
	std::vector<std::vector<Square> > _downDiagA8h1MovesList;
};

}

#endif
