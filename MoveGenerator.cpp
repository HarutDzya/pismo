#include "MoveGenerator.h"
#include "PositionState.h"
#include "PossibleMoves.h"
#include "MemPool.h"

#include <assert.h>

namespace pismo
{

MoveGenerator* MoveGenerator::_instance = 0;

MoveGenerator* MoveGenerator::instance()
{
	if (!_instance) {
		_instance = new MoveGenerator();
	}

	return _instance;
}

void MoveGenerator::destroy()
{
	delete _instance;
	_instance = 0;
}

MoveGenerator::MoveGenerator() :
	_possibleMoves(new PossibleMoves())
{

}

void MoveGenerator::generateWhiteMoves(const PositionState& pos, MovesArray& generatedMoves)
{
	assert(pos.whiteToPlay());
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.getBoard()[from / 8][from % 8]) {
			case PAWN_WHITE:
				generatePawnMoves((Square) from, WHITE, pos, generatedMoves);
				break;
			case KNIGHT_WHITE:
				generateKnightMoves((Square) from, pos, generatedMoves);
				break;
			case BISHOP_WHITE:
				generateDiagA1h8Moves((Square) from, pos, generatedMoves);
				generateDiagA8h1Moves((Square) from, pos, generatedMoves);
				break;
			case ROOK_WHITE:
				generateRankMoves((Square) from, pos, generatedMoves);
				generateFileMoves((Square) from, pos, generatedMoves);
				break;
			case QUEEN_WHITE:
				generateDiagA1h8Moves((Square) from, pos, generatedMoves);
				generateDiagA8h1Moves((Square) from, pos, generatedMoves);
				generateRankMoves((Square) from, pos, generatedMoves);
				generateFileMoves((Square) from, pos, generatedMoves);
				break;
			case KING_WHITE:
				generateKingMoves((Square) from, pos, generatedMoves);
			default:
				break;
		}
	}
}

void MoveGenerator::generateBlackMoves(const PositionState& pos, MovesArray& generatedMoves)
{
	assert(!pos.whiteToPlay());
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.getBoard()[from / 8][from % 8]) {
			case PAWN_BLACK:
				generatePawnMoves((Square) from, BLACK, pos, generatedMoves);
				break;
			case KNIGHT_BLACK:
				generateKnightMoves((Square) from, pos, generatedMoves);
				break;
			case BISHOP_BLACK:
				generateDiagA1h8Moves((Square) from, pos, generatedMoves);
				generateDiagA8h1Moves((Square) from, pos, generatedMoves);
				break;
			case ROOK_BLACK:
				generateRankMoves((Square) from, pos, generatedMoves);
				generateFileMoves((Square) from, pos, generatedMoves);
				break;
			case QUEEN_BLACK:
				generateDiagA1h8Moves((Square) from, pos, generatedMoves);
				generateDiagA8h1Moves((Square) from, pos, generatedMoves);
				generateRankMoves((Square) from, pos, generatedMoves);
				generateFileMoves((Square) from, pos, generatedMoves);
				break;
			case KING_BLACK:
				generateKingMoves((Square) from, pos, generatedMoves);
			default:
				break;
		}
	}
}

// Generates all legal pawn moves for color clr from square from
// and adds these moves to generatedMoves data member
void MoveGenerator::generatePawnMoves(Square from, Color clr, const PositionState& pos, MovesArray& generatedMoves)
{
	if (clr == WHITE) {
		const std::vector<Square>& whitePawnMoves =  _possibleMoves->possibleWhitePawnMoves(from);
		if(from >= A7 && from <= H7) {
			for (std::size_t moveCount = 0; moveCount < whitePawnMoves.size(); ++moveCount) {
				MoveInfo currentMove = {from, whitePawnMoves[moveCount], KNIGHT_WHITE};
				if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
					generatedMoves.push_back(currentMove);
					currentMove.promoted = BISHOP_WHITE;
					generatedMoves.push_back(currentMove);
					currentMove.promoted = ROOK_WHITE;
					generatedMoves.push_back(currentMove);
					currentMove.promoted = QUEEN_WHITE;
					generatedMoves.push_back(currentMove);
				}
		       }
		}
		else {
			for (std::size_t moveCount = 0; moveCount < whitePawnMoves.size(); ++moveCount) {
				MoveInfo currentMove = {from, whitePawnMoves[moveCount], ETY_SQUARE};
				if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
					generatedMoves.push_back(currentMove);
				}
			}
		}
	}
	else {
		const std::vector<Square>& blackPawnMoves = _possibleMoves->possibleBlackPawnMoves(from);
		if(from >= A2 && from <= H2) {
			for (std::size_t moveCount = 0; moveCount < blackPawnMoves.size(); ++moveCount) {
				MoveInfo currentMove = {from, blackPawnMoves[moveCount], KNIGHT_BLACK};
				if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
					generatedMoves.push_back(currentMove);
					currentMove.promoted = BISHOP_BLACK;
					generatedMoves.push_back(currentMove);
					currentMove.promoted = ROOK_BLACK;
					generatedMoves.push_back(currentMove);
					currentMove.promoted = QUEEN_BLACK;
					generatedMoves.push_back(currentMove);
				}
		       }
		}
		else {
			for (std::size_t moveCount = 0; moveCount < blackPawnMoves.size(); ++moveCount) {
				MoveInfo currentMove = {from, blackPawnMoves[moveCount], ETY_SQUARE};
				if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
					generatedMoves.push_back(currentMove);
				}
			}
		}
	}
}

// Generates all legal knight moves from square from
// and adds these moves to generatedMoves data member
void MoveGenerator::generateKnightMoves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& knightMoves = _possibleMoves->possibleKnightMoves(from);
	for (std::size_t moveCount  = 0; moveCount < knightMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, knightMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
	}
}

// Generates all legal king moves from square from
// and adds these moves to generatedMoves data member
void MoveGenerator::generateKingMoves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& kingMoves = _possibleMoves->possibleKingMoves(from);
	for (std::size_t moveCount  = 0; moveCount < kingMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, kingMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
	}
}

// Generates all legal rank moves from square from
// and adds these moves to generatedMoves data member
// This function should be used for rook and queen
void MoveGenerator::generateRankMoves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& leftRankMoves = _possibleMoves->possibleLeftRankMoves(from);
	for(std::size_t moveCount = 0; moveCount < leftRankMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, leftRankMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& rightRankMoves = _possibleMoves->possibleRightRankMoves(from);
	for(std::size_t moveCount = 0; moveCount < rightRankMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, rightRankMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal file moves from square from
// and adds these moves to generatedMoves data member
// This function should be used for rook and queen
void MoveGenerator::generateFileMoves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& upFileMoves = _possibleMoves->possibleUpFileMoves(from);
	for(std::size_t moveCount = 0; moveCount < upFileMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, upFileMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downFileMoves = _possibleMoves->possibleDownFileMoves(from);
	for(std::size_t moveCount = 0; moveCount < downFileMoves.size(); ++moveCount) {
		MoveInfo currentMove = {from, downFileMoves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal A1H8 diagonal moves from square from
// and adds these moves to generatedMoves data member
// This function should be used for bishop and queen
void MoveGenerator::generateDiagA1h8Moves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& upDiagA1h8Moves = _possibleMoves->possibleUpDiagA1h8Moves(from);
	for(std::size_t moveCount = 0; moveCount < upDiagA1h8Moves.size(); ++moveCount) {
		MoveInfo currentMove = {from, upDiagA1h8Moves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA1h8Moves = _possibleMoves->possibleDownDiagA1h8Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA1h8Moves.size(); ++moveCount) {
		MoveInfo currentMove = {from, downDiagA1h8Moves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal A8H1 diagonal moves from square from
// and adds these moves to generatedMoves data member
// This function should be used for bishop and queen
void MoveGenerator::generateDiagA8h1Moves(Square from, const PositionState& pos, MovesArray& generatedMoves)
{
	const std::vector<Square>& upDiagA8h1Moves = _possibleMoves->possibleUpDiagA8h1Moves(from);
	for(std::size_t moveCount = 0; moveCount < upDiagA8h1Moves.size(); ++moveCount) {
		MoveInfo currentMove = {from, upDiagA8h1Moves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA8h1Moves = _possibleMoves->possibleDownDiagA8h1Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA8h1Moves.size(); ++moveCount) {
		MoveInfo currentMove = {from, downDiagA8h1Moves[moveCount], ETY_SQUARE};
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

MoveGenerator::~MoveGenerator()
{
	delete _possibleMoves;
}

}
