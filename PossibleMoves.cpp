#include "PossibleMoves.h"

namespace pismo
{

PossibleMoves::PossibleMoves()
{
	initWhitePawnMoves();
	initBlackPawnMoves();
	initKnightMoves();
	initKingMoves();
	initLeftRankMoves();
	initRightRankMoves();
	initUpFileMoves();
	initDownFileMoves();
	initUpDiagA1h8Moves();
	initDownDiagA1h8Moves();
	initUpDiagA8h1Moves();
	initDownDiagA8h1Moves();
}

const std::vector<Square>& PossibleMoves::possibleWhitePawnMoves(Square from) const
{
	return _whitePawnMovesList[from - A2];
}

const std::vector<Square>& PossibleMoves::possibleBlackPawnMoves(Square from) const
{
	return _blackPawnMovesList[from - A2];
}

const std::vector<Square>& PossibleMoves::possibleKnightMoves(Square from) const
{
	return _knightMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleKingMoves(Square from) const
{
	return _kingMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleLeftRankMoves(Square from) const
{
	return _leftRankMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleRightRankMoves(Square from) const
{
	return _rightRankMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleUpFileMoves(Square from) const
{
	return _upFileMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleDownFileMoves(Square from) const
{
	return _downFileMovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleUpDiagA1h8Moves(Square from) const
{
	return _upDiagA1h8MovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleDownDiagA1h8Moves(Square from) const
{
	return _downDiagA1h8MovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleUpDiagA8h1Moves(Square from) const
{
	return _upDiagA8h1MovesList[from];
}

const std::vector<Square>& PossibleMoves::possibleDownDiagA8h1Moves(Square from) const
{
	return _downDiagA8h1MovesList[from];
}

void PossibleMoves::initWhitePawnMoves()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		std::vector<Square> pawnMovesSquare;
		pawnMovesSquare.push_back((Square) (sq + 8));
		if ((sq % 8) > (A1 % 8)) { 
			pawnMovesSquare.push_back((Square) (sq + 7));
		}
		if ((sq % 8) < (H1 % 8)) {
			pawnMovesSquare.push_back((Square) (sq + 9));
		}
		if (sq >= A2 && sq <= H2) {
			pawnMovesSquare.push_back((Square) (sq + 16));
		}
		_whitePawnMovesList.push_back(pawnMovesSquare);
	}
}

void PossibleMoves::initBlackPawnMoves()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		std::vector<Square> pawnMovesSquare;
		pawnMovesSquare.push_back((Square) (sq - 8));
		if ((sq % 8) < (H1 % 8)) {
			pawnMovesSquare.push_back((Square) (sq - 7));
		}
		if ((sq % 8) > (A1 % 8)) { 
			pawnMovesSquare.push_back((Square) (sq - 9));
		}
		if (sq >= A7 && sq <= H7) {
			pawnMovesSquare.push_back((Square) (sq - 16));
		}
		_blackPawnMovesList.push_back(pawnMovesSquare);
	}
}

void PossibleMoves::initKnightMoves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> knightMovesSquare;
		if ((sq / 8 >= A3 / 8) && (sq % 8 >= B3 % 8)) {
			knightMovesSquare.push_back((Square) (sq - 17));
		}
		if ((sq / 8 >= A2 / 8) && (sq % 8 >= C2 % 8)) {
			knightMovesSquare.push_back((Square) (sq - 10));
		}
		if ((sq / 8 <= A7 / 8) && (sq % 8 >= C7 % 8)) {
			knightMovesSquare.push_back((Square) (sq + 6));
		}
		if ((sq / 8 <= A6 / 8) && (sq % 8 >= B6 % 8)) {
			knightMovesSquare.push_back((Square) (sq + 15));
		}
		if ((sq / 8 <= A6 / 8) && (sq % 8 <= G6 % 8)) {
			knightMovesSquare.push_back((Square) (sq + 17));
		}
		if ((sq / 8 <= A7 / 8) && (sq % 8 <= F7 % 8)) {
			knightMovesSquare.push_back((Square) (sq + 10));
		}
		if ((sq / 8 >= A2 / 8) && (sq % 8 <= F2 % 8)) {
			knightMovesSquare.push_back((Square) (sq - 6));
		}
		if ((sq / 8 >= A3 / 8) && (sq % 8 <= G3 % 8)) {
			knightMovesSquare.push_back((Square) (sq - 15));
		}		
		_knightMovesList.push_back(knightMovesSquare);
	}
}

void PossibleMoves::initKingMoves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> kingMovesSquare;
		if (sq / 8 >= A2 / 8) {
			if (sq % 8 <= G2 % 8) {
				kingMovesSquare.push_back((Square) (sq - 7));
			}
			kingMovesSquare.push_back((Square) (sq - 8));
			if (sq % 8 >= B2 % 8) {
				kingMovesSquare.push_back((Square) (sq - 9));
			}
		}
		if (sq % 8 >= B2 % 8) {
			kingMovesSquare.push_back((Square) (sq - 1));
		}
		if (sq / 8 <= A7 / 8) {
			if (sq % 8 >= B2 % 8) {
				kingMovesSquare.push_back((Square) (sq + 7));
			}
			kingMovesSquare.push_back((Square) (sq + 8));
			if (sq % 8 <= G2 % 8) {
				kingMovesSquare.push_back((Square) (sq + 9));
			}
		}
		if (sq % 8 <= G2 % 8) {
			kingMovesSquare.push_back((Square) (sq + 1));
		}
		if (sq == E1) {
			kingMovesSquare.push_back(C1);
			kingMovesSquare.push_back(G1);
		}
		if (sq == E8) {
			kingMovesSquare.push_back(C8);
			kingMovesSquare.push_back(G8);
		}
		_kingMovesList.push_back(kingMovesSquare);
	}
}

// The left rank moves are initialized starting from one left of the current square 
// position until the left end of the rank  
void PossibleMoves::initLeftRankMoves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> leftRankMovesSquare;
		for (int sqTo = sq - 1 ; sqTo >= A1 && (sq / 8 - sqTo / 8) == 0; --sqTo) {
		       leftRankMovesSquare.push_back((Square) sqTo);
		}
		_leftRankMovesList.push_back(leftRankMovesSquare);
	}
}

// The right rank moves are initialized starting from one right of the current square 
// position until the right end of the rank  
void PossibleMoves::initRightRankMoves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> rightRankMovesSquare;
		for (unsigned int sqTo = sq + 1 ; sqTo <= H8 && (sqTo / 8 - sq / 8) == 0; ++sqTo) {
		       rightRankMovesSquare.push_back((Square) sqTo);
		}
		_rightRankMovesList.push_back(rightRankMovesSquare);
	}
}

// The up file moves are initialized starting from one square up of the current
// square position until the end of the file
void PossibleMoves::initUpFileMoves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> upFileMovesSquare;
		for (unsigned int sqTo = sq + 8; sqTo <= H8; sqTo += 8) {
			upFileMovesSquare.push_back((Square) sqTo);
		}
		_upFileMovesList.push_back(upFileMovesSquare);
	}
}

// The down file moves are initialized starting from one square down of the current
// square position until the end of the file
void PossibleMoves::initDownFileMoves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> downFileMovesSquare;
		for (int sqTo = sq - 8; sqTo >= A1; sqTo -= 8) {
			downFileMovesSquare.push_back((Square) sqTo);
		}
		_downFileMovesList.push_back(downFileMovesSquare);
	}
}

// The up diagonal A1H8 moves are initialized starting from one square up 
// on diagonal of the current square position until the end of diagonal  
void PossibleMoves::initUpDiagA1h8Moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> upDiagA1h8MovesSquare;
		for (unsigned int sqTo = sq + 9; sqTo <= H8 && (sqTo / 8 - sq / 8) == (sqTo - sq) / 9; sqTo += 9) {
		       upDiagA1h8MovesSquare.push_back((Square) sqTo);
		}
 		_upDiagA1h8MovesList.push_back(upDiagA1h8MovesSquare);
	}
}

// The down diagonal A1H8 moves are initialized starting from one square 
// down on diagonal of the current square position until the end of diagonal
void PossibleMoves::initDownDiagA1h8Moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> downDiagA1h8MovesSquare;
		for (int sqTo = sq - 9; sqTo >= A1 && (sq / 8 - sqTo / 8) == (sq - sqTo) / 9; sqTo -= 9) {
		       downDiagA1h8MovesSquare.push_back((Square) sqTo);
		}
 		_downDiagA1h8MovesList.push_back(downDiagA1h8MovesSquare);
	}
}

// The up diagonal A8H1 moves are initialized starting from one square up
// on diagonal of the current square position until the end of diagonal
void PossibleMoves::initUpDiagA8h1Moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> upDiagA8h1MovesSquare;
		for (unsigned int sqTo = sq + 7; sqTo <= H8 && (sqTo / 8 - sq / 8) == (sqTo - sq) / 7; sqTo += 7) {
			upDiagA8h1MovesSquare.push_back((Square) sqTo);
		}
		_upDiagA8h1MovesList.push_back(upDiagA8h1MovesSquare);
	}
}

// The down diagonal A8H1 moves are initialized starting from one square
// down on diagonal of the current square position until the end of diagonal
void PossibleMoves::initDownDiagA8h1Moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> downDiagA8h1MovesSquare;
		for (int sqTo = sq - 7; sqTo >= A1 && (sq / 8 - sqTo / 8) == (sq - sqTo) / 7; sqTo -= 7) {
			downDiagA8h1MovesSquare.push_back((Square) sqTo);
		}
		_downDiagA8h1MovesList.push_back(downDiagA8h1MovesSquare);
	}
}
}
