#include "MoveGenerator.h"
#include "PositionState.h"
#include "PossibleMoves.h"
#include "BitboardImpl.h"
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
	_bitboardImpl(BitboardImpl::instance()),
	_availableMoves(new MovesArray(MAX_AVAILABLE_MOVES))
{

}

void MoveGenerator::prepareMoveGeneration(const PositionState& pos, SearchType type, const MoveInfo& transTableMove, MovesArray& generatedMoves)	
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
	_positionState = &pos;
	_availableMoves = &generatedMoves;
}

MoveInfo MoveGenerator::getTopMove()
{	
	if (_currentMovePos < _availableMovesSize && equal((*_availableMoves)[_currentMovePos], _cachedMove)) {
		++_currentMovePos;
	}
	if (_currentMovePos == _availableMovesSize) {
		switch(_searchType) {
			case USUAL_SEARCH:
				generateMovesForUsualSearch();
				break;
			case EVASION_SEARCH:
				generateMovesForEvasionSearch();
				break;
			case QUITE_SEARCH:
				generateMovesForQuiteSearch();
				break;
		}
	}

	if ( _currentMovePos < _availableMovesSize) {
		return (*_availableMoves)[_currentMovePos++];
	}

	return MoveInfo();
}

bool MoveGenerator::equal(const MoveInfo& first, const MoveInfo& second) const
{
	return (first.from == second.from) && (first.to == second.to) && (first.promoted == second.promoted);
}

void MoveGenerator::generateMovesForUsualSearch()
{
	_currentMovePos = 0;
	_availableMovesSize = 0;
	switch(_nextStage) {
		case CAPTURING_MOVES:
			generateCapturingMoves();
			sortCapturingMoves();
			_nextStage = CHECKING_MOVES;
			break;
		case CHECKING_MOVES:
			generateCheckingMoves();
			sortCheckingMoves();
			_nextStage = QUITE_MOVES;
			break;
		case QUITE_MOVES:
			generateQuiteMoves();
			sortQuiteMoves();
			_nextStage = SEARCH_FINISHED;
			break;
		case SEARCH_FINISHED:
		default:
			break;
	}
}

void MoveGenerator::generateMovesForEvasionSearch()
{
	_currentMovePos = 0;
	_availableMovesSize = 0;
	switch(_nextStage) {
		case EVASION_MOVES:
			generateEvasionMoves();
			sortEvasionMoves();
			_nextStage = SEARCH_FINISHED;
			break;
		case SEARCH_FINISHED:
		default:
			break;
	}
}

void MoveGenerator::generateMovesForQuiteSearch()
{
	_currentMovePos = 0;
	_availableMovesSize = 0;	
	switch(_nextStage) {
		case CAPTURING_MOVES:
			generateCapturingMoves();
			sortCapturingMoves();
			_nextStage = CHECKING_MOVES;
			break;
		case CHECKING_MOVES:
			generateCheckingMoves();
			sortCheckingMoves();
			_nextStage = SEARCH_FINISHED;
			break;
		case SEARCH_FINISHED:
		default:
			break;
	}
}

void MoveGenerator::generateEvasionMoves()
{
	generateKingEvasionMoves();

	if (!_positionState->isDoubleCheck()) {
		Bitboard absolutePinsPos = _positionState->absolutePinsPos();

		while (absolutePinsPos) {
			Square to = (Square) _bitboardImpl->lsb(absolutePinsPos);
			MoveType type = _positionState->getBoard()[to / 8][to % 8] == ETY_SQUARE ? NORMAL_MOVE : CAPTURE_MOVE;
			generatePawnsEvasionMoves(to, type);
			generateKnightsEvasionMoves(to, type);
			generateBishopsEvasionMoves(to, type);
			generateRooksEvasionMoves(to, type);
			generateQueensEvasionMoves(to, type);

			absolutePinsPos &= (absolutePinsPos - 1);
		}
	}
}


void MoveGenerator::generateKingEvasionMoves() 
{
	if (_positionState->whiteToPlay()) {
		Square from = _positionState->whiteKingPosition();
		Bitboard moveBoard = (_bitboardImpl->kingAttackFrom(from)) & ~(_positionState->whitePieces());
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (squareToBitboard[to] & _positionState->blackPieces()) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Square from = _positionState->blackKingPosition();
		Bitboard moveBoard = (_bitboardImpl->kingAttackFrom(from)) & ~(_positionState->blackPieces());
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (squareToBitboard[to] & _positionState->whitePieces()) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all possible pawns move to square to
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generatePawnsEvasionMoves(Square to, MoveType type)
{
	if (_positionState->whiteToPlay()) {
		generatePawnsWhiteEvasionMoves(to, type);
	}
	else {
		generatePawnsBlackEvasionMoves(to, type);
	}
}

// Generates all possible white pawns move to square to
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generatePawnsWhiteEvasionMoves(Square to, MoveType type)
{
	Bitboard pawnsWhitePos = _positionState->getPiecePos()[PAWN_WHITE];
	if (type == CAPTURE_MOVE) {
		Bitboard attackingPawnsPos = _bitboardImpl->pawnsWhiteAttackTo(to, pawnsWhitePos);
		while (attackingPawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(attackingPawnsPos);
			if (to >= A8) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			}
			attackingPawnsPos &= (attackingPawnsPos - 1);
		}
	}
	else {
		Bitboard movingPawnPos = _bitboardImpl->pawnWhiteMovesTo(to, _positionState->occupiedSquares(), pawnsWhitePos);
		if (movingPawnPos) {
			Square from = (Square) _bitboardImpl->lsb(movingPawnPos);
			if (to >= A8) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE);
			}
			else if (to - from == 16) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
		}
	}
		
	Square enPassantTarget = _positionState->enPassantTarget();
	if (to + 8 == enPassantTarget || to == enPassantTarget) {
		Bitboard enPassantCapturePawnsPos = _bitboardImpl->pawnsWhiteAttackTo(enPassantTarget, pawnsWhitePos);
		while (enPassantCapturePawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(enPassantCapturePawnsPos);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE);
			enPassantCapturePawnsPos &= (enPassantCapturePawnsPos - 1);
		}
	}
}

// Generates all possible black pawns move to square to
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generatePawnsBlackEvasionMoves(Square to, MoveType type)
{
	Bitboard pawnsBlackPos = _positionState->getPiecePos()[PAWN_BLACK];
	if (type == CAPTURE_MOVE) {
		Bitboard attackingPawnsPos = _bitboardImpl->pawnsBlackAttackTo(to, pawnsBlackPos);
		while (attackingPawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(attackingPawnsPos);
			if (to <= H1) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			}
			attackingPawnsPos &= (attackingPawnsPos - 1);
		}
	}
	else {
		Bitboard movingPawnPos = _bitboardImpl->pawnBlackMovesTo(to, _positionState->occupiedSquares(), pawnsBlackPos);
		if (movingPawnPos) {
			Square from = (Square) _bitboardImpl->lsb(movingPawnPos);
			if (to <= H1) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE);
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE);
			}
			else if (from - to == 16) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
		}
	}
		
	Square enPassantTarget = _positionState->enPassantTarget();
	if (to == enPassantTarget + 8 || to == enPassantTarget) {
		Bitboard enPassantCapturePawnsPos = _bitboardImpl->pawnsBlackAttackTo(enPassantTarget, pawnsBlackPos);
		while (enPassantCapturePawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(enPassantCapturePawnsPos);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE);
			enPassantCapturePawnsPos &= (enPassantCapturePawnsPos - 1);
		}
	}
}

// Generates all possible knights move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateKnightsEvasionMoves(Square to, MoveType type)
{
	Bitboard knightsPos = _positionState->whiteToPlay() ? _positionState->getPiecePos()[KNIGHT_WHITE] : _positionState->getPiecePos()[KNIGHT_BLACK];
	Bitboard movingKnightsPos = _bitboardImpl->knightsAttackTo(to, knightsPos);
	while (movingKnightsPos) {
		Square from = (Square) _bitboardImpl->lsb(movingKnightsPos);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, type);
		movingKnightsPos &= (movingKnightsPos - 1);
	}
}

// Generates all possible bishops move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateBishopsEvasionMoves(Square to, MoveType type)
{
	Bitboard bishopsPos = _positionState->whiteToPlay() ? _positionState->getPiecePos()[BISHOP_WHITE] : _positionState->getPiecePos()[BISHOP_BLACK];
	Bitboard movingBishopsPos = _bitboardImpl->bishopsAttackTo(to,_positionState->occupiedSquares(), bishopsPos);
	while (movingBishopsPos) {
		Square from = (Square) _bitboardImpl->lsb(movingBishopsPos);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, type);
		movingBishopsPos &= (movingBishopsPos - 1);
	}
}

// Generates all possible rooks move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateRooksEvasionMoves(Square to, MoveType type)
{
	Bitboard rooksPos = _positionState->whiteToPlay() ? _positionState->getPiecePos()[ROOK_WHITE] : _positionState->getPiecePos()[ROOK_BLACK];
	Bitboard movingRooksPos = _bitboardImpl->rooksAttackTo(to,_positionState->occupiedSquares(), rooksPos);
	while (movingRooksPos) {
		Square from = (Square) _bitboardImpl->lsb(movingRooksPos);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, type);
		movingRooksPos &= (movingRooksPos - 1);
	}
}

// Generates all possible queens move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateQueensEvasionMoves(Square to, MoveType type)
{
	Bitboard queensPos = _positionState->whiteToPlay() ? _positionState->getPiecePos()[QUEEN_WHITE] : _positionState->getPiecePos()[QUEEN_BLACK];
	Bitboard movingQueensPos = _bitboardImpl->queensAttackTo(to,_positionState->occupiedSquares(), queensPos);
	while (movingQueensPos) {
		Square from = (Square) _bitboardImpl->lsb(movingQueensPos);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, type);
		movingQueensPos &= (movingQueensPos - 1);
	}
}

void MoveGenerator::generateCapturingMoves()
{
	if(_positionState->whiteToPlay()) {
		Bitboard pawnsWhitePos = _positionState->getPiecePos()[PAWN_WHITE];
		while (pawnsWhitePos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsWhitePos);
			generatePawnWhiteCapturingMoves(from);
			pawnsWhitePos &= (pawnsWhitePos - 1);
		}
	
		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_WHITE];
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightCapturingMoves(from);
			knightsPos &= (knightsPos - 1);
		}
		
		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_WHITE];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookCapturingMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_WHITE];
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopCapturingMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}

		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_WHITE];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenCapturingMoves(from);
			queensPos &= (queensPos - 1);
		}

		generateKingCapturingMoves(_positionState->whiteKingPosition());
	}
	else {
		Bitboard pawnsBlackPos = _positionState->getPiecePos()[PAWN_BLACK];
		while (pawnsBlackPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsBlackPos);
			generatePawnBlackCapturingMoves(from);
			pawnsBlackPos &= (pawnsBlackPos - 1);
		}
	
		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_BLACK];
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightCapturingMoves(from);
			knightsPos &= (knightsPos - 1);
		}
		
		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_BLACK];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookCapturingMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_BLACK];
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopCapturingMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}

		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_BLACK];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenCapturingMoves(from);
			queensPos &= (queensPos - 1);
		}

		generateKingCapturingMoves(_positionState->blackKingPosition());
	}
}

// Generates all white pawn capturing moves from square from
void MoveGenerator::generatePawnWhiteCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->blackPieces();
	if (from >= A7) {
		Bitboard promotionBoard = (_bitboardImpl->pawnWhiteAttackFrom(from) & opponentPieces) | _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares());
		while(promotionBoard) {
			Square to = (Square) _bitboardImpl->lsb(promotionBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE);
			promotionBoard &= (promotionBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->pawnWhiteAttackFrom(from) & opponentPieces;
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			moveBoard &= (moveBoard - 1);
		}

		Square enPassantTarget = _positionState->enPassantTarget();
		if (enPassantTarget != INVALID_SQUARE && (_bitboardImpl->pawnWhiteAttackFrom(from) & squareToBitboard[enPassantTarget])) {
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE);
		}
	}
}

// Generates all black pawn capturing moves from square from
void MoveGenerator::generatePawnBlackCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whitePieces();
	if (from <= H2) {
		Bitboard promotionBoard = (_bitboardImpl->pawnBlackAttackFrom(from) & opponentPieces) | _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares());
		while(promotionBoard) {
			Square to = (Square) _bitboardImpl->lsb(promotionBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE);
			promotionBoard &= (promotionBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->pawnBlackAttackFrom(from) & opponentPieces;
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
			moveBoard &= (moveBoard - 1);
		}

		Square enPassantTarget = _positionState->enPassantTarget();
		if (enPassantTarget != INVALID_SQUARE && (_bitboardImpl->pawnBlackAttackFrom(from) & squareToBitboard[enPassantTarget])) {
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE);
		}
	}
}

// Generates all knight capturing moves from square from
void MoveGenerator::generateKnightCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whiteToPlay() ? _positionState->blackPieces() : _positionState->whitePieces();
	Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & opponentPieces;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all rook capturing moves from square from
void MoveGenerator::generateRookCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whiteToPlay() ? _positionState->blackPieces() : _positionState->whitePieces();
	Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & opponentPieces;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all bishop capturing moves from square from
void MoveGenerator::generateBishopCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whiteToPlay() ? _positionState->blackPieces() : _positionState->whitePieces();
	Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & opponentPieces;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all queen capturing moves from square from
void MoveGenerator::generateQueenCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whiteToPlay() ? _positionState->blackPieces() : _positionState->whitePieces();
	Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & opponentPieces;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all king capturing moves from square from
void MoveGenerator::generateKingCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whiteToPlay() ? _positionState->blackPieces() : _positionState->whitePieces();
	Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & opponentPieces;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

void MoveGenerator::generateCheckingMoves()
{
	if (_positionState->whiteToPlay()) {
		Bitboard pawnsPos = _positionState->getPiecePos()[PAWN_WHITE];
		while (pawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsPos);
			generatePawnDirectCheckingMoves(from);
			pawnsPos &= (pawnsPos - 1);
		}

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_WHITE];
		if (squareToBitboard[_positionState->blackKingPosition()] & WHITE_SQUARES_MASK) {
			knightsPos &= BLACK_SQUARES_MASK;
		}
		else {
			knightsPos &= WHITE_SQUARES_MASK;
		}
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightDirectCheckingMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_WHITE];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookDirectCheckingMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_WHITE];
		if (squareToBitboard[_positionState->blackKingPosition()] & WHITE_SQUARES_MASK) {
			bishopsPos &= WHITE_SQUARES_MASK;
		}
		else {
			bishopsPos &= BLACK_SQUARES_MASK;
		}
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopDirectCheckingMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}
		
		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_WHITE];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenDirectCheckingMoves(from);
			queensPos &= (queensPos - 1);
		}

		generateKingWhiteCastlingCheckingMoves(_positionState->whiteKingPosition());
		generateWhiteDiscoveredCheckingMoves();
	}
	else {
		Bitboard pawnsPos = _positionState->getPiecePos()[PAWN_BLACK];
		while (pawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsPos);
			generatePawnDirectCheckingMoves(from);
			pawnsPos &= (pawnsPos - 1);
		}

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_BLACK];
		if (squareToBitboard[_positionState->whiteKingPosition()] & WHITE_SQUARES_MASK) {
			knightsPos &= BLACK_SQUARES_MASK;
		}
		else {
			knightsPos &= WHITE_SQUARES_MASK;
		}
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightDirectCheckingMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_BLACK];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookDirectCheckingMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_BLACK];
		if (squareToBitboard[_positionState->whiteKingPosition()] & WHITE_SQUARES_MASK) {
			bishopsPos &= WHITE_SQUARES_MASK;
		}
		else {
			bishopsPos &= BLACK_SQUARES_MASK;
		}
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopDirectCheckingMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}
		
		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_BLACK];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenDirectCheckingMoves(from);
			queensPos &= (queensPos - 1);
		}

		generateKingBlackCastlingCheckingMoves(_positionState->blackKingPosition());
		generateBlackDiscoveredCheckingMoves();
	}
}

// Generates all pawn direct checking moves from square from 
void MoveGenerator::generatePawnDirectCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		if (from < A7) {
			// Promotion moves are not considered here, because they were already considered in generatePawnWhiteCapturingMoves
			Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares()) & _positionState->getDirectCheck()[PAWN_WHITE];
			if (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (to - from == 16) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
			}
		}
	}
	else {
		if (from > H2) {
			// Promotion moves are not considered here, because they were already considered in generatePawnBlackCapturingMoves
			Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares()) & _positionState->getDirectCheck()[PAWN_BLACK];
			if (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (from - to == 16) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
			}
		}
	}
}

// Generates all knight direct checking moves from square from
void MoveGenerator::generateKnightDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ? (_positionState->getDirectCheck()[KNIGHT_WHITE] & ~_positionState->occupiedSquares()) : (_positionState->getDirectCheck()[KNIGHT_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all rook direct checking moves from square from
void MoveGenerator::generateRookDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ? (_positionState->getDirectCheck()[ROOK_WHITE] & ~_positionState->occupiedSquares()) : (_positionState->getDirectCheck()[ROOK_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all bishop direct checking moves from square from
void MoveGenerator::generateBishopDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ? (_positionState->getDirectCheck()[BISHOP_WHITE] & ~_positionState->occupiedSquares()) : (_positionState->getDirectCheck()[BISHOP_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all queen direct checking moves from square from
void MoveGenerator::generateQueenDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ? (_positionState->getDirectCheck()[QUEEN_WHITE] & ~_positionState->occupiedSquares()) : (_positionState->getDirectCheck()[QUEEN_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all white king castling moves which check opponent king from square from
void MoveGenerator::generateKingWhiteCastlingCheckingMoves(Square from) 
{
	if (from == E1) {
		if (_positionState->whiteLeftCastling() && !(WHITE_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->blackKingPosition() > H1) {
				if (squareToBitboard[D1] & _positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E1] & _positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
		if (_positionState->whiteRightCastling() && !(WHITE_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->blackKingPosition() > H1) {
				if (squareToBitboard[F1] & _positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E1] & _positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
	}
}

// Generates all black king castling moves which check opponent king from square from
void MoveGenerator::generateKingBlackCastlingCheckingMoves(Square from)
{
	if (from == E8) {
		if (_positionState->blackLeftCastling() && !(BLACK_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->whiteKingPosition() < A8) {
				if (squareToBitboard[D8] & _positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E8] & _positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
		if (_positionState->blackRightCastling() && !(BLACK_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->whiteKingPosition() < A8) {
				if (squareToBitboard[F8] & _positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E8] & _positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
	}
}

void MoveGenerator::generateWhiteDiscoveredCheckingMoves()
{
	_discCheckPiecePos = 0;
	if (_positionState->rankDiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->rankDiscCheck().smallPinPos & _positionState->whitePieces();
		if (movingPiece) {
			Square from = (Square) _bitboardImpl->lsb(movingPiece);
			generateDiscoveredCheckingMoves(from, _positionState->rankDiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->rankDiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->rankDiscCheck().bigPinPos & _positionState->whitePieces();
		if (movingPiece) {
			Square from = (Square) _bitboardImpl->lsb(movingPiece);
			generateDiscoveredCheckingMoves(from, _positionState->rankDiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->fileDiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->fileDiscCheck().smallPinPos & _positionState->whitePiecesTranspose();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareTranspose((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->fileDiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->fileDiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->fileDiscCheck().bigPinPos & _positionState->whitePiecesTranspose();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareTranspose((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->fileDiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA1h8DiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA1h8DiscCheck().smallPinPos & _positionState->whitePiecesDiagA1h8();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA1h8((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA1h8DiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA1h8DiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA1h8DiscCheck().bigPinPos & _positionState->whitePiecesDiagA1h8();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA1h8((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA1h8DiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA8h1DiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA8h1DiscCheck().smallPinPos & _positionState->whitePiecesDiagA8h1();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA8h1((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA8h1DiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA8h1DiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA8h1DiscCheck().bigPinPos & _positionState->whitePiecesDiagA8h1();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA8h1((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA8h1DiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}
}

void MoveGenerator::generateBlackDiscoveredCheckingMoves()
{
	_discCheckPiecePos = 0;
	if (_positionState->rankDiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->rankDiscCheck().smallPinPos & _positionState->blackPieces();
		if (movingPiece) {
			Square from = (Square) _bitboardImpl->lsb(movingPiece);
			generateDiscoveredCheckingMoves(from, _positionState->rankDiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->rankDiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->rankDiscCheck().bigPinPos & _positionState->blackPieces();
		if (movingPiece) {
			Square from = (Square) _bitboardImpl->lsb(movingPiece);
			generateDiscoveredCheckingMoves(from, _positionState->rankDiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->fileDiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->fileDiscCheck().smallPinPos & _positionState->blackPiecesTranspose();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareTranspose((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->fileDiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->fileDiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->fileDiscCheck().bigPinPos & _positionState->blackPiecesTranspose();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareTranspose((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->fileDiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA1h8DiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA1h8DiscCheck().smallPinPos & _positionState->blackPiecesDiagA1h8();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA1h8((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA1h8DiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA1h8DiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA1h8DiscCheck().bigPinPos & _positionState->blackPiecesDiagA1h8();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA1h8((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA1h8DiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA8h1DiscCheck().smallSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA8h1DiscCheck().smallPinPos & _positionState->blackPiecesDiagA8h1();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA8h1((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA8h1DiscCheck().smallSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}

	if (_positionState->diagA8h1DiscCheck().bigSlidingPiecePos != INVALID_SQUARE) {
		Bitboard movingPiece = _positionState->diagA8h1DiscCheck().bigPinPos & _positionState->blackPiecesDiagA8h1();
		if (movingPiece) {
			Square from = _bitboardImpl->squareToSquareA8h1((Square) _bitboardImpl->lsb(movingPiece));
			generateDiscoveredCheckingMoves(from, _positionState->diagA8h1DiscCheck().bigSlidingPiecePos);
			_discCheckPiecePos |= squareToBitboard[from];
		}
	}
}

void MoveGenerator::generateDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay()) {
		switch (_positionState->getBoard()[from / 8][from % 8]) {
			case PAWN_WHITE:
				generatePawnDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case KNIGHT_WHITE:
				generateKnightDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case BISHOP_WHITE:
				generateBishopDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case ROOK_WHITE:
				generateRookDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case KING_WHITE:
				generateKingDiscoveredCheckingMoves(from, slidingPiecePos);
			// Queen cannot open discovered check, therefore is not considered here
			default:
				assert(false);
		}
	}
	else {
		switch (_positionState->getBoard()[from / 8][from % 8]) {
			case PAWN_BLACK:
				generatePawnDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case KNIGHT_BLACK:
				generateKnightDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case BISHOP_BLACK:
				generateBishopDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case ROOK_BLACK:
				generateRookDiscoveredCheckingMoves(from, slidingPiecePos);
				break;
			case KING_BLACK:
				generateKingDiscoveredCheckingMoves(from, slidingPiecePos);
			// Queen cannot open discovered check, therefore is not considered here
			default:
				assert(false);
		}
	}
}

// Generates all pawn discovered checking moves from square from
// when sliding piece is at the position slidingPiecePos; square from 
// should be on the ray from slidingPiecePos to opponent king position
void MoveGenerator::generatePawnDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay() && from < A7) {
		// Promotion moves are not considered here, because they were already considered in generatePawnWhiteCapturingMoves
		Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->blackKingPosition()) & ~_positionState->getDirectCheck()[PAWN_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (to - from == 16) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
	else if (!_positionState->whiteToPlay() && from > H2) {
		// Promotion moves are not considered here, because they were already considered in generatePawnBlackCapturingMoves
		Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->whiteKingPosition()) & ~_positionState->getDirectCheck()[PAWN_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (from - to == 16) {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all knight discovered checking moves from square from
// when sliding piece is at the position slidingPiecePos; square from 
// should be on the ray from slidingPiecePos to opponent king position
void MoveGenerator::generateKnightDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->blackKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->whiteKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all rook discovered checking moves from square from
// when sliding piece is at the position slidingPiecePos; square from 
// should be on the ray from slidingPiecePos to opponent king position
void MoveGenerator::generateRookDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->blackKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->whiteKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all bishop discovered checking moves from square from
// when sliding piece is at the position slidingPiecePos; square from 
// should be on the ray from slidingPiecePos to opponent king position
void MoveGenerator::generateBishopDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->blackKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->whiteKingPosition()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all king discovered checking moves from square from
// when sliding piece is at the position slidingPiecePos; square from
// should be on the ray from slidingPiecePos to opponent king position
void MoveGenerator::generateKingDiscoveredCheckingMoves(Square from, Square slidingPiecePos)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->blackKingPosition()) & ~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->blackKingPosition());
	   while (moveBoard) {
		   Square to = (Square) _bitboardImpl->lsb(moveBoard);
		   (*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		   moveBoard &= (moveBoard - 1);
	   }
	}
	else {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_bitboardImpl->getSquaresBetween(slidingPiecePos, _positionState->whiteKingPosition()) & ~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->whiteKingPosition());
	   while (moveBoard) {
		   Square to = (Square) _bitboardImpl->lsb(moveBoard);
		   (*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		   moveBoard &= (moveBoard - 1);
	   }
	}
}

void MoveGenerator::generateQuiteMoves()
{
	if (_positionState->whiteToPlay()) {
		Bitboard pawnsPos = _positionState->getPiecePos()[PAWN_WHITE];
		while (pawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsPos);
			generatePawnWhiteQuiteMoves(from);
			pawnsPos &= (pawnsPos - 1);
		}

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_WHITE] & ~_discCheckPiecePos;
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightQuiteMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_WHITE] & ~_discCheckPiecePos;
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookQuiteMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_WHITE] & ~_discCheckPiecePos;
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopQuiteMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}
		
		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_WHITE];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenQuiteMoves(from);
			queensPos &= (queensPos - 1);
		}
		
		generateKingWhiteQuiteMoves(_positionState->whiteKingPosition());
	}
	else {
		Bitboard pawnsPos = _positionState->getPiecePos()[PAWN_BLACK];
		while (pawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsPos);
			generatePawnBlackQuiteMoves(from);
			pawnsPos &= (pawnsPos - 1);
		}

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_BLACK] & ~_discCheckPiecePos;
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightQuiteMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_BLACK] & ~_discCheckPiecePos;
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookQuiteMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_BLACK] & ~_discCheckPiecePos;
		while (bishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(bishopsPos);
			generateBishopQuiteMoves(from);
			bishopsPos &= (bishopsPos - 1);
		}
		
		Bitboard queensPos = _positionState->getPiecePos()[QUEEN_BLACK];
		while (queensPos) {
			Square from = (Square) _bitboardImpl->lsb(queensPos);
			generateQueenQuiteMoves(from);
			queensPos &= (queensPos - 1);
		}
		
		generateKingBlackQuiteMoves(_positionState->blackKingPosition());
	}
}

// Generates all white pawn quite moves from square from
// that is all non-promotion, non-capturing and non-checking moves
void MoveGenerator::generatePawnWhiteQuiteMoves(Square from)
{
	if (from < A7) {
		Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares()) & ~_positionState->getDirectCheck()[PAWN_WHITE];
		if (squareToBitboard[from] & ~_discCheckPiecePos) {
			while (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (to - from == 16) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
				moveBoard &= (moveBoard - 1);
			}
		}
		else {
			while (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (_bitboardImpl->getSquaresBetween(to, _positionState->blackKingPosition()) & _bitboardImpl->getSquaresBetween(from, _positionState->blackKingPosition())) {
					if (to - from == 16) {
						(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
					}
					else {
						(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
					}
					moveBoard &= (moveBoard - 1);
				}
				else {
					return;
				}
			}
		}
	}
}

// Generates all black pawn quite moves from square from
// that is all non-promotion, non-capturing and non-checking moves
void MoveGenerator::generatePawnBlackQuiteMoves(Square from)
{
	if (from > H2) {
		Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares()) & ~_positionState->getDirectCheck()[PAWN_BLACK];
		if (squareToBitboard[from] & ~_discCheckPiecePos) {
			while (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (from - to == 16) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
				moveBoard &= (moveBoard - 1);
			}
		}
		else {
			while (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (_bitboardImpl->getSquaresBetween(to, _positionState->whiteKingPosition()) & _bitboardImpl->getSquaresBetween(from, _positionState->whiteKingPosition())) {
					if (from - to == 16) {
						(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
					}
					else {
						(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
					}
					moveBoard &= (moveBoard - 1);
				}
				else {
					return;
				}
			}
		}
	}
}

// Generates all knight quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateKnightQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all rook quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateRookQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all bishop quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateBishopQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all queen quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateQueenQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[QUEEN_WHITE];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & ~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[QUEEN_BLACK];
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all white king quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateKingWhiteQuiteMoves(Square from)
{
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->blackKingPosition());
	if (squareToBitboard[from] & _discCheckPiecePos) {
		moveBoard &= _bitboardImpl->getSlidingPieceMoves(_positionState->blackKingPosition());
	}
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}

	if (from == E1) {
		if (_positionState->whiteLeftCastling() && !(WHITE_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->blackKingPosition() > H1) {
				if (squareToBitboard[D1] & ~_positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E1] & ~_positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
		if (_positionState->whiteRightCastling() && !(WHITE_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->blackKingPosition() > H1) {
				if (squareToBitboard[F1] & ~_positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E1] & ~_positionState->getDirectCheck()[ROOK_WHITE]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G1, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
	}
}

// Generates all black king quite moves from square from
// that is all non-capturing and non-checking moves
void MoveGenerator::generateKingBlackQuiteMoves(Square from)
{
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->whiteKingPosition());
	if (squareToBitboard[from] & _discCheckPiecePos) {
		moveBoard &= _bitboardImpl->getSlidingPieceMoves(_positionState->whiteKingPosition());
	}
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}

	if (from == E8) {
		if (_positionState->blackLeftCastling() && !(BLACK_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->whiteKingPosition() < A8) {
				if (squareToBitboard[D8] & ~_positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E8] & ~_positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, C8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
		if (_positionState->blackRightCastling() && !(BLACK_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			if (_positionState->whiteKingPosition() < A8) {
				if (squareToBitboard[F8] & ~_positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
			else {
				if (squareToBitboard[E8] & ~_positionState->getDirectCheck()[ROOK_BLACK]) {
					(*_availableMoves)[_availableMovesSize++] = MoveInfo(from, G8, ETY_SQUARE, CASTLING_MOVE);
				}
			}
		}
	}
}

void MoveGenerator::sortCapturingMoves()
{
}

void MoveGenerator::sortCheckingMoves()
{
}

void MoveGenerator::sortEvasionMoves()
{
}

void MoveGenerator::sortQuiteMoves()
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
