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
	_possibleMoves(new PossibleMoves()),
	_availableMoves(new MovesArray(MAX_AVAILABLE_MOVES))
{

}

void MoveGenerator::prepareMoveGeneration(const PositionState& pos, SearchType type, const MoveInfo& transTableMove)	
{
	_searchType = type;
	_cachedMove = transTableMove; 
	switch (_searchType) {
		case USUAL_SEARCH:
			_nextStage = CAPTURING_MOVES;
			break;
		case EVASION_SEARCH:
			_nextStage = EVASION_MOVES;
			break;
		case QUITE_SEARCH:
			_nextStage = CAPTURING_MOVES;
			break;
		default:
			assert(false);
	}
	_availableMovesSize = 0;
	if (!pos.isDoubleCheck()) {
		generateAvailableMoves(pos);
	}
	else {
		generateKingMoves(pos.movingKingPosition(), pos);
	}
}


void MoveGenerator::generateAvailableMoves(const PositionState& pos)
{
	if (pos.whiteToPlay()) {
		for (unsigned int from = A1; from <= H8; ++from) {
			switch(pos.getBoard()[from / 8][from % 8]) {
				case PAWN_WHITE:
					generatePawnMoves((Square) from, pos);
					break;
				case KNIGHT_WHITE:
					generateKnightMoves((Square) from, pos);
					break;
				case BISHOP_WHITE:
					generateDiagA1h8Moves((Square) from, pos);
					generateDiagA8h1Moves((Square) from, pos);
					break;
				case ROOK_WHITE:
					generateRankMoves((Square) from, pos);
					generateFileMoves((Square) from, pos);
					break;
				case QUEEN_WHITE:
					generateDiagA1h8Moves((Square) from, pos);
					generateDiagA8h1Moves((Square) from, pos);
					generateRankMoves((Square) from, pos);
					generateFileMoves((Square) from, pos);
					break;
				case KING_WHITE:
					generateKingMoves((Square) from, pos);
				default:
					break;
			}
		}
	}
	else {
		for (unsigned int from = A1; from <= H8; ++from) {
			switch(pos.getBoard()[from / 8][from % 8]) {
				case PAWN_BLACK:
					generatePawnMoves((Square) from, pos);
					break;
				case KNIGHT_BLACK:
					generateKnightMoves((Square) from, pos);
					break;
				case BISHOP_BLACK:
					generateDiagA1h8Moves((Square) from, pos);
					generateDiagA8h1Moves((Square) from, pos);
					break;
				case ROOK_BLACK:
					generateRankMoves((Square) from, pos);
					generateFileMoves((Square) from, pos);
					break;
				case QUEEN_BLACK:
					generateDiagA1h8Moves((Square) from, pos);
					generateDiagA8h1Moves((Square) from, pos);
					generateRankMoves((Square) from, pos);
					generateFileMoves((Square) from, pos);
					break;
				case KING_BLACK:
					generateKingMoves((Square) from, pos);
				default:
					assert(false);
			}
		}
	}
}
	
// Generates all available pawn moves without checking any validity
// and adds these moves to _availableMoves data member
void MoveGenerator::generatePawnMoves(Square from, const PositionState& pos)
{
	if (pos.whiteToPlay()) {
		const std::vector<Square>& whitePawnMoves =  _possibleMoves->possibleWhitePawnMoves(from);
		if(from >= A7 && from <= H7) {
			for (std::size_t moveCount = 0; moveCount < whitePawnMoves.size(); ++moveCount) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, whitePawnMoves[moveCount], KNIGHT_WHITE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, whitePawnMoves[moveCount], BISHOP_WHITE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, whitePawnMoves[moveCount], ROOK_WHITE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, whitePawnMoves[moveCount], QUEEN_WHITE);
			}
		}
		else {
			for (std::size_t moveCount = 0; moveCount < whitePawnMoves.size(); ++moveCount) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, whitePawnMoves[moveCount], ETY_SQUARE);
			}
		}
	}
	else {
		const std::vector<Square>& blackPawnMoves = _possibleMoves->possibleBlackPawnMoves(from);
		if(from >= A2 && from <= H2) {
			for (std::size_t moveCount = 0; moveCount < blackPawnMoves.size(); ++moveCount) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, blackPawnMoves[moveCount], KNIGHT_BLACK);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, blackPawnMoves[moveCount], BISHOP_BLACK);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, blackPawnMoves[moveCount], ROOK_BLACK);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, blackPawnMoves[moveCount], QUEEN_BLACK);
		     }
		}
		else {
			for (std::size_t moveCount = 0; moveCount < blackPawnMoves.size(); ++moveCount) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, blackPawnMoves[moveCount], ETY_SQUARE);
			}
		}
	}
}

// Generates all available knight available moves from square from without checking any validity
// and adds these moves to _availableMoves data member
void MoveGenerator::generateKnightMoves(Square from, const PositionState& pos)
{
	const std::vector<Square>& knightMoves = _possibleMoves->possibleKnightMoves(from);
	for (std::size_t moveCount  = 0; moveCount < knightMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, knightMoves[moveCount], ETY_SQUARE);
	}
}

// Generates all availalble king moves from square from without checking any validity
// and adds these moves to _availableMoves data member
void MoveGenerator::generateKingMoves(Square from, const PositionState& pos)
{
	const std::vector<Square>& kingMoves = _possibleMoves->possibleKingMoves(from);
	for (std::size_t moveCount  = 0; moveCount < kingMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, kingMoves[moveCount], ETY_SQUARE);
	}
}

// Generates all available rank moves from square from without checking any validity
// and adds these moves to _availableMoves data member
// This function should be used for rook and queen
void MoveGenerator::generateRankMoves(Square from, const PositionState& pos)
{
	const std::vector<Square>& leftRankMoves = _possibleMoves->possibleLeftRankMoves(from);
	for(std::size_t moveCount = 0; moveCount < leftRankMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, leftRankMoves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[leftRankMoves[moveCount] / 8][leftRankMoves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& rightRankMoves = _possibleMoves->possibleRightRankMoves(from);
	for(std::size_t moveCount = 0; moveCount < rightRankMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, rightRankMoves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[rightRankMoves[moveCount] / 8][rightRankMoves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all available file moves from square from without checking any validity
// and adds these moves to _availableMoves data member
// This function should be used for rook and queen
void MoveGenerator::generateFileMoves(Square from, const PositionState& pos)
{
	const std::vector<Square>& upFileMoves = _possibleMoves->possibleUpFileMoves(from);
	for(std::size_t moveCount = 0; moveCount < upFileMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, upFileMoves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[upFileMoves[moveCount] / 8][upFileMoves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downFileMoves = _possibleMoves->possibleDownFileMoves(from);
	for(std::size_t moveCount = 0; moveCount < downFileMoves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, downFileMoves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[downFileMoves[moveCount] / 8][downFileMoves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all available A1H8 diagonal moves from square from without checking any validity
// and adds these moves to _availableMoves data member
// This function should be used for bishop and queen
void MoveGenerator::generateDiagA1h8Moves(Square from, const PositionState& pos)
{
	const std::vector<Square>& upDiagA1h8Moves = _possibleMoves->possibleUpDiagA1h8Moves(from);
	for(std::size_t moveCount = 0; moveCount < upDiagA1h8Moves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, upDiagA1h8Moves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[upDiagA1h8Moves[moveCount] / 8][upDiagA1h8Moves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA1h8Moves = _possibleMoves->possibleDownDiagA1h8Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA1h8Moves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, downDiagA1h8Moves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[downDiagA1h8Moves[moveCount] / 8][downDiagA1h8Moves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all available A8H1 diagonal moves from square from without checking any validity
// and adds these moves to _availableMoves data member
// This function should be used for bishop and queen
void MoveGenerator::generateDiagA8h1Moves(Square from, const PositionState& pos)
{
	const std::vector<Square>& upDiagA8h1Moves = _possibleMoves->possibleUpDiagA8h1Moves(from);
	for(std::size_t moveCount = 0; moveCount < upDiagA8h1Moves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, upDiagA8h1Moves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[upDiagA8h1Moves[moveCount] / 8][upDiagA8h1Moves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA8h1Moves = _possibleMoves->possibleDownDiagA8h1Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA8h1Moves.size(); ++moveCount) {
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, downDiagA8h1Moves[moveCount], ETY_SQUARE);
		if (pos.getBoard()[downDiagA8h1Moves[moveCount] / 8][downDiagA8h1Moves[moveCount] % 8] != ETY_SQUARE) {
			break;
		}
	}
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
				MoveInfo currentMove(from, whitePawnMoves[moveCount], KNIGHT_WHITE);
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
				MoveInfo currentMove(from, whitePawnMoves[moveCount], ETY_SQUARE);
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
				MoveInfo currentMove(from, blackPawnMoves[moveCount], KNIGHT_BLACK);
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
				MoveInfo currentMove(from, blackPawnMoves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, knightMoves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, kingMoves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, leftRankMoves[moveCount], ETY_SQUARE);
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& rightRankMoves = _possibleMoves->possibleRightRankMoves(from);
	for(std::size_t moveCount = 0; moveCount < rightRankMoves.size(); ++moveCount) {
		MoveInfo currentMove(from, rightRankMoves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, upFileMoves[moveCount], ETY_SQUARE);
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downFileMoves = _possibleMoves->possibleDownFileMoves(from);
	for(std::size_t moveCount = 0; moveCount < downFileMoves.size(); ++moveCount) {
		MoveInfo currentMove(from, downFileMoves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, upDiagA1h8Moves[moveCount], ETY_SQUARE);
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA1h8Moves = _possibleMoves->possibleDownDiagA1h8Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA1h8Moves.size(); ++moveCount) {
		MoveInfo currentMove(from, downDiagA1h8Moves[moveCount], ETY_SQUARE);
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
		MoveInfo currentMove(from, upDiagA8h1Moves[moveCount], ETY_SQUARE);
		if (pos.moveIsPseudoLegal(currentMove) && pos.pseudoMoveIsLegalMove(currentMove)) {
			generatedMoves.push_back(currentMove);
		}
		if (pos.getBoard()[currentMove.to / 8][currentMove.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& downDiagA8h1Moves = _possibleMoves->possibleDownDiagA8h1Moves(from);
	for(std::size_t moveCount = 0; moveCount < downDiagA8h1Moves.size(); ++moveCount) {
		MoveInfo currentMove(from, downDiagA8h1Moves[moveCount], ETY_SQUARE);
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
