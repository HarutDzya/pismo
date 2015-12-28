#include "MoveGenerator.h"
#include "PositionState.h"
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
	_bitboardImpl(BitboardImpl::instance()),
	_gainSEE(new int32_t[MAX_SEARCH_DEPTH])
{

}

void MoveGenerator::prepareMoveGeneration(SearchType type, const MoveInfo& transTableMove, uint16_t depth)	
{
	_moveGenInfo = MemPool::getMoveGenInfo(depth);
	_moveGenInfo->_searchType = type;
	_moveGenInfo->_cachedMove = transTableMove; 
	switch (_moveGenInfo->_searchType) {
		case USUAL_SEARCH:
			_moveGenInfo->_nextStage = CAPTURING_MOVES;
			break;
		case EVASION_SEARCH:
			_moveGenInfo->_nextStage = EVASION_MOVES;
			break;
		case QUIESCENCE_SEARCH:
			_moveGenInfo->_nextStage = CAPTURING_MOVES;
			break;
		default:
			assert(false);
	}
	_moveGenInfo->_currentMovePos = 0;
	_moveGenInfo->_availableMovesSize = 0;
}

void MoveGenerator::generatePerftMoves(const PositionState& pos, uint16_t depth)
{
  _positionState = &pos;
  _moveGenInfo = MemPool::getMoveGenInfo(depth);
  _moveGenInfo->_currentMovePos = 0;
  _moveGenInfo->_availableMovesSize = 0;

  if (_positionState->kingUnderCheck())
  {
     generateEvasionMoves();
  }
  else
  {
    generateCapturingMoves();
    generateQuiteMoves();
  }
}

MoveInfo MoveGenerator::getTopMove(const PositionState& pos, uint16_t depth)
{
	_moveGenInfo = MemPool::getMoveGenInfo(depth);
	assert(_moveGenInfo);
	_positionState = &pos;
	if (_moveGenInfo->_currentMovePos < _moveGenInfo->_availableMovesSize && 
			equal((_moveGenInfo->_availableMoves)[_moveGenInfo->_currentMovePos], _moveGenInfo->_cachedMove)) {
		++(_moveGenInfo->_currentMovePos);
	}
	if (_moveGenInfo->_currentMovePos == _moveGenInfo->_availableMovesSize) {
		switch(_moveGenInfo->_searchType) {
			case USUAL_SEARCH:
				generateMovesForUsualSearch();
				break;
			case EVASION_SEARCH:
				generateMovesForEvasionSearch();
				break;
			case QUIESCENCE_SEARCH:
				generateMovesForQuiescenceSearch();
				break;
		}
	}

	if ( _moveGenInfo->_currentMovePos < _moveGenInfo->_availableMovesSize) {
		return (_moveGenInfo->_availableMoves)[(_moveGenInfo->_currentMovePos)++];
	}

	return MoveInfo();
}

bool MoveGenerator::equal(const MoveInfo& first, const MoveInfo& second) const
{
	return (first.from == second.from) && (first.to == second.to) && (first.promoted == second.promoted);
}

void MoveGenerator::generateMovesForUsualSearch()
{
	_moveGenInfo->_currentMovePos = 0;
	_moveGenInfo->_availableMovesSize = 0;
	switch(_moveGenInfo->_nextStage) {
		case CAPTURING_MOVES:
			generateCapturingMoves();
			sortCapturingMoves();
			_moveGenInfo->_nextStage = QUITE_MOVES;
			if (_moveGenInfo->_availableMovesSize != 0) {
				break;
			}
		case QUITE_MOVES:
			generateQuiteMoves();
			sortQuiteMoves();
			_moveGenInfo->_nextStage = SEARCH_FINISHED;
			if (_moveGenInfo->_availableMovesSize != 0) {
				break;
			}
		case SEARCH_FINISHED:
		default:
			break;
	}
}

void MoveGenerator::generateMovesForEvasionSearch()
{
	_moveGenInfo->_currentMovePos = 0;
	_moveGenInfo->_availableMovesSize = 0;
	switch(_moveGenInfo->_nextStage) {
		case EVASION_MOVES:
			generateEvasionMoves();
			sortEvasionMoves();
			_moveGenInfo->_nextStage = SEARCH_FINISHED;
			if (_moveGenInfo->_availableMovesSize != 0) {
				break;
			}
		case SEARCH_FINISHED:
		default:
			break;
	}
}

void MoveGenerator::generateMovesForQuiescenceSearch()
{
	_moveGenInfo->_currentMovePos = 0;
	_moveGenInfo->_availableMovesSize = 0;	
	switch(_moveGenInfo->_nextStage) {
		case CAPTURING_MOVES:
			generateCapturingMoves();
			sortCapturingMoves();
			_moveGenInfo->_nextStage = CHECKING_MOVES;
			if (_moveGenInfo->_availableMovesSize != 0) {
				break;
			}
		case CHECKING_MOVES:
			generateCheckingMoves();
			sortCheckingMoves();
			_moveGenInfo->_nextStage = SEARCH_FINISHED;
			if (_moveGenInfo->_availableMovesSize != 0) {
				break;
			}
		case SEARCH_FINISHED:
		default:
			break;
	}
}

// Generates all evasion moves and should be called
// only if moving side king is under check
void MoveGenerator::generateEvasionMoves()
{
	generateKingEvasionMoves();

	if (!_positionState->isDoubleCheck()) {
		Bitboard absolutePinsPos = _positionState->absolutePinsPos();

		while (absolutePinsPos) {
			Square to = (Square) _bitboardImpl->lsb(absolutePinsPos);
			MoveType type = _positionState->getBoard()[mRank(to)][mFile(to)] == ETY_SQUARE ? NORMAL_MOVE : CAPTURE_MOVE;
			generatePawnsEvasionMoves(to, type);
			generateKnightsEvasionMoves(to, type);
			generateBishopsEvasionMoves(to, type);
			generateRooksEvasionMoves(to, type);
			generateQueensEvasionMoves(to, type);

			absolutePinsPos &= (absolutePinsPos - 1);
		}
	}
}

// Generates all king evasion moves, that is the moves
// which eliminate the check of the king
void MoveGenerator::generateKingEvasionMoves() 
{
	if (_positionState->whiteToPlay()) {
		Square from = _positionState->whiteKingPosition();
		Bitboard moveBoard = (_bitboardImpl->kingAttackFrom(from)) & ~(_positionState->whitePieces());
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (squareToBitboard[to] & _positionState->blackPieces()) {
				uint16_t value = -PIECE_VALUES[KING_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
				//TODO: Make the assignements of the components of MoveInfo in lieu, rather than making temporary object
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
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
				uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[KING_BLACK];
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
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
		uint16_t value = -PIECE_VALUES[PAWN_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
		while (attackingPawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(attackingPawnsPos);
			if (to >= A8) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE, value);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			}
			attackingPawnsPos &= (attackingPawnsPos - 1);
		}
	}
	else {
		Bitboard movingPawnPos = _bitboardImpl->pawnWhiteMovesTo(to, _positionState->occupiedSquares(), pawnsWhitePos);
		if (movingPawnPos) {
			Square from = (Square) _bitboardImpl->lsb(movingPawnPos);
			if (to >= A8) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE);
			}
			else if (to - from == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
		}
	}
		
	Square enPassantTarget = _positionState->enPassantTarget();
	if (to + 8 == enPassantTarget || to == enPassantTarget) {
		Bitboard enPassantCapturePawnsPos = _bitboardImpl->pawnsWhiteAttackTo(enPassantTarget, pawnsWhitePos);
		while (enPassantCapturePawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(enPassantCapturePawnsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE, 0);
			// For en passant capture move value is always 0, because pawn captures pawn
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
		uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[PAWN_BLACK];
		while (attackingPawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(attackingPawnsPos);
			if (to <= H1) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE, value);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE, value);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			}
			attackingPawnsPos &= (attackingPawnsPos - 1);
		}
	}
	else {
		Bitboard movingPawnPos = _bitboardImpl->pawnBlackMovesTo(to, _positionState->occupiedSquares(), pawnsBlackPos);
		if (movingPawnPos) {
			Square from = (Square) _bitboardImpl->lsb(movingPawnPos);
			if (to <= H1) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE);
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE);
			}
			else if (from - to == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
		}
	}
		
	Square enPassantTarget = _positionState->enPassantTarget();
	if (to == enPassantTarget + 8 || to == enPassantTarget) {
		Bitboard enPassantCapturePawnsPos = _bitboardImpl->pawnsBlackAttackTo(enPassantTarget, pawnsBlackPos);
		while (enPassantCapturePawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(enPassantCapturePawnsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE, 0);
			// For en passant capture move value is always 0, because pawn captures pawn
			enPassantCapturePawnsPos &= (enPassantCapturePawnsPos - 1);
		}
	}
}

// Generates all possible knights move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateKnightsEvasionMoves(Square to, MoveType type)
{
	if (_positionState->whiteToPlay()) {
		Bitboard movingKnightsPos = _bitboardImpl->knightsAttackTo(to, _positionState->getPiecePos()[KNIGHT_WHITE]);
		uint16_t value = (type == CAPTURE_MOVE) ? 
			-PIECE_VALUES[KNIGHT_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] : 0;
		while (movingKnightsPos) {
			Square from = (Square) _bitboardImpl->lsb(movingKnightsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingKnightsPos &= (movingKnightsPos - 1);
		}
	}
	else {
		Bitboard movingKnightsPos = _bitboardImpl->knightsAttackTo(to, _positionState->getPiecePos()[KNIGHT_BLACK]);
		uint16_t value = (type == CAPTURE_MOVE) ? 
			PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[KNIGHT_BLACK] : 0;
		while (movingKnightsPos) {
			Square from = (Square) _bitboardImpl->lsb(movingKnightsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingKnightsPos &= (movingKnightsPos - 1);
		}
	}
}

// Generates all possible bishops move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateBishopsEvasionMoves(Square to, MoveType type)
{
	if (_positionState->whiteToPlay()) {
		Bitboard movingBishopsPos = _bitboardImpl->bishopsAttackTo(to, 
				_positionState->occupiedSquares(), _positionState->getPiecePos()[BISHOP_WHITE]);
		uint16_t value = (type == CAPTURE_MOVE) ? 
			-PIECE_VALUES[BISHOP_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] : 0;	
		while (movingBishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(movingBishopsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingBishopsPos &= (movingBishopsPos - 1);
		}
	}
else {
		Bitboard movingBishopsPos = _bitboardImpl->bishopsAttackTo(to, 
				_positionState->occupiedSquares(), _positionState->getPiecePos()[BISHOP_BLACK]);
		uint16_t value = (type == CAPTURE_MOVE) ? 
			PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[BISHOP_BLACK] : 0;	
		while (movingBishopsPos) {
			Square from = (Square) _bitboardImpl->lsb(movingBishopsPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingBishopsPos &= (movingBishopsPos - 1);
		}
	}
}

// Generates all possible rooks move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateRooksEvasionMoves(Square to, MoveType type)
{
	if (_positionState->whiteToPlay()) {
		Bitboard movingRooksPos = _bitboardImpl->rooksAttackTo(to,
				_positionState->occupiedSquares(), _positionState->getPiecePos()[ROOK_WHITE]);
		uint16_t value = (type == CAPTURE_MOVE) ?
			-PIECE_VALUES[ROOK_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] : 0;
		while (movingRooksPos) {
			Square from = (Square) _bitboardImpl->lsb(movingRooksPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingRooksPos &= (movingRooksPos - 1);
		}
	}
	else {
		Bitboard movingRooksPos = _bitboardImpl->rooksAttackTo(to,
				_positionState->occupiedSquares(), _positionState->getPiecePos()[ROOK_BLACK]);
		uint16_t value = (type == CAPTURE_MOVE) ?
			PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[ROOK_BLACK] : 0;
		while (movingRooksPos) {
			Square from = (Square) _bitboardImpl->lsb(movingRooksPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingRooksPos &= (movingRooksPos - 1);
		}
	}
}

// Generates all possible queens move to square to 
// which are evasion moves; type shows whether it is normal or capture move
void MoveGenerator::generateQueensEvasionMoves(Square to, MoveType type)
{
	if (_positionState->whiteToPlay()) {
		Bitboard movingQueensPos = _bitboardImpl->queensAttackTo(to,
				_positionState->occupiedSquares(), _positionState->getPiecePos()[QUEEN_WHITE]);
		uint16_t value = (type == CAPTURE_MOVE) ?
			-PIECE_VALUES[QUEEN_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] : 0;
		while (movingQueensPos) {
			Square from = (Square) _bitboardImpl->lsb(movingQueensPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingQueensPos &= (movingQueensPos - 1);
		}
	}
	else {
		Bitboard movingQueensPos = _bitboardImpl->queensAttackTo(to,
				_positionState->occupiedSquares(), _positionState->getPiecePos()[QUEEN_BLACK]);
		uint16_t value = (type == CAPTURE_MOVE) ?
			PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[QUEEN_BLACK] : 0;
		while (movingQueensPos) {
			Square from = (Square) _bitboardImpl->lsb(movingQueensPos);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, type, value);
			movingQueensPos &= (movingQueensPos - 1);
		}
	}
}

// Generates all capturing and promotion moves
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

// Generates all white pawn capturing and promotion moves from square from
void MoveGenerator::generatePawnWhiteCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->blackPieces();
	if (from >= A7) {
		Bitboard promotionBoard = (_bitboardImpl->pawnWhiteAttackFrom(from) & opponentPieces) |
		   	_bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares());
		while(promotionBoard) {
			Square to = (Square) _bitboardImpl->lsb(promotionBoard);
			uint16_t value = -PIECE_VALUES[PAWN_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_WHITE, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_WHITE, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_WHITE, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_WHITE, PROMOTION_MOVE, value);
			promotionBoard &= (promotionBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->pawnWhiteAttackFrom(from) & opponentPieces;
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[PAWN_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}

		Square enPassantTarget = _positionState->enPassantTarget();
		if (enPassantTarget != INVALID_SQUARE && (_bitboardImpl->pawnWhiteAttackFrom(from) & squareToBitboard[enPassantTarget])) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE, 0);
			// For en passant capture move value is always 0, becuase pawn captures pawn
		}
	}
}

// Generates all black pawn capturing and promotion moves from square from
void MoveGenerator::generatePawnBlackCapturingMoves(Square from)
{
	Bitboard opponentPieces = _positionState->whitePieces();
	if (from <= H2) {
		Bitboard promotionBoard = (_bitboardImpl->pawnBlackAttackFrom(from) & opponentPieces) |
		   	_bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares());
		while(promotionBoard) {
			Square to = (Square) _bitboardImpl->lsb(promotionBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[PAWN_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, KNIGHT_BLACK, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, BISHOP_BLACK, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ROOK_BLACK, PROMOTION_MOVE, value);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, QUEEN_BLACK, PROMOTION_MOVE, value);
			promotionBoard &= (promotionBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->pawnBlackAttackFrom(from) & opponentPieces;
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[PAWN_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}

		Square enPassantTarget = _positionState->enPassantTarget();
		if (enPassantTarget != INVALID_SQUARE && (_bitboardImpl->pawnBlackAttackFrom(from) & squareToBitboard[enPassantTarget])) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, enPassantTarget, ETY_SQUARE, EN_PASSANT_CAPTURE, 0);
			// FOr en passant capture move value is always 0, becuase pawn captures pawn
		}
	}
}

// Generates all knight capturing moves from square from
void MoveGenerator::generateKnightCapturingMoves(Square from)
{
	if (_positionState->whiteToPlay()){
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & _positionState->blackPieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[KNIGHT_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & _positionState->whitePieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[KNIGHT_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all rook capturing moves from square from
void MoveGenerator::generateRookCapturingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & _positionState->blackPieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[ROOK_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & _positionState->whitePieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[ROOK_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all bishop capturing moves from square from
void MoveGenerator::generateBishopCapturingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & _positionState->blackPieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[BISHOP_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & _positionState->whitePieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[BISHOP_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all queen capturing moves from square from
void MoveGenerator::generateQueenCapturingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & _positionState->blackPieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[QUEEN_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & _positionState->whitePieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[QUEEN_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all king capturing moves from square from
void MoveGenerator::generateKingCapturingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & _positionState->blackPieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = -PIECE_VALUES[KING_WHITE] - PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & _positionState->whitePieces();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			uint16_t value = PIECE_VALUES[_positionState->getBoard()[mRank(to)][mFile(to)]] + PIECE_VALUES[KING_BLACK];
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, CAPTURE_MOVE, value);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all checking moves which are non-capturing and non-promotion moves
// Checking moves due to castling are not considered here
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

		generateBlackDiscoveredCheckingMoves();
	}
}

// Generates all pawn direct checking moves from square from
// Capturing and promotion moves are not considered here
// becuase they were already generated in generateCapturingMoves
void MoveGenerator::generatePawnDirectCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		if (from < A7) {
			// Promotion moves are not considered here, because they were already considered in generatePawnWhiteCapturingMoves
			Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares()) &
			   	_positionState->getDirectCheck()[PAWN_WHITE];
			if (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (to - from == 16) {
					(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
			}
		}
	}
	else {
		if (from > H2) {
			// Promotion moves are not considered here, because they were already considered in generatePawnBlackCapturingMoves
			Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares()) &
			   	_positionState->getDirectCheck()[PAWN_BLACK];
			if (moveBoard) {
				Square to = (Square) _bitboardImpl->lsb(moveBoard);
				if (from - to == 16) {
					(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
				}
				else {
					(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
				}
			}
		}
	}
}

// Generates all knight direct checking moves from square from
// Capturing moves are not considered here, because they were
// already considered in generateCapturingMoves
void MoveGenerator::generateKnightDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ?
	   	(_positionState->getDirectCheck()[KNIGHT_WHITE] & ~_positionState->occupiedSquares()) :
	   	(_positionState->getDirectCheck()[KNIGHT_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all rook direct checking moves from square from
// Capturing moves are not considered here, because they were
// already considered in generateCapturingMoves
void MoveGenerator::generateRookDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ?
	   	(_positionState->getDirectCheck()[ROOK_WHITE] & ~_positionState->occupiedSquares()) :
	   	(_positionState->getDirectCheck()[ROOK_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all bishop direct checking moves from square from
// Capturing moves are not considered here, because they were
// already considered in generateCapturingMoves
void MoveGenerator::generateBishopDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ?
	   	(_positionState->getDirectCheck()[BISHOP_WHITE] & ~_positionState->occupiedSquares()) :
	   	(_positionState->getDirectCheck()[BISHOP_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generate all queen direct checking moves from square from
// Capturing moves are not considered here, because they were
// already considered in generateCapturingMoves
void MoveGenerator::generateQueenDirectCheckingMoves(Square from)
{
	Bitboard directCheckPos = _positionState->whiteToPlay() ?
	   	(_positionState->getDirectCheck()[QUEEN_WHITE] & ~_positionState->occupiedSquares()) :
	   	(_positionState->getDirectCheck()[QUEEN_BLACK] & ~_positionState->occupiedSquares());
	Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) & directCheckPos;
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}
}

// Generates all discovered checking moves for white side
// Direct checking moves are not considered here, becuase they were
// already generated in generateDirectCheckingMoves
// Capturing and promotion moves are not considered here, because they were
// already generated in generateCapturingMoves
void MoveGenerator::generateWhiteDiscoveredCheckingMoves()
{
	Bitboard discPiecePos = _positionState->discPiecePos() & _positionState->whitePieces();
	while (discPiecePos) {
		Square from = (Square) _bitboardImpl->lsb(discPiecePos);
		switch (_positionState->getBoard()[mRank(from)][mFile(from)]) {
			case PAWN_WHITE:
				generatePawnDiscoveredCheckingMoves(from);
				break;
			case KNIGHT_WHITE:
				generateKnightDiscoveredCheckingMoves(from);
				break;
			case BISHOP_WHITE:
				generateBishopDiscoveredCheckingMoves(from);
				break;
			case ROOK_WHITE:
				generateRookDiscoveredCheckingMoves(from);
				break;
			case KING_WHITE:
				generateKingDiscoveredCheckingMoves(from);
				break;
			// Queen cannot open discovered check, therefore is not considered here
			default:
				assert(false);
		}
		discPiecePos &= (discPiecePos - 1);
	}
}

// Generates all discovered checking moves for black side
// Direct checking moves are not considered here, becuase they were
// already generated in generateDirectCheckingMoves
// Capturing and promotion moves are not considered here, because they were
// already generated in generateCapturingMoves
void MoveGenerator::generateBlackDiscoveredCheckingMoves()
{
	Bitboard discPiecePos = _positionState->discPiecePos() & _positionState->blackPieces();
	while (discPiecePos) {
		Square from = (Square) _bitboardImpl->lsb(discPiecePos);
		switch (_positionState->getBoard()[mRank(from)][mFile(from)]) {
			case PAWN_BLACK:
				generatePawnDiscoveredCheckingMoves(from);
				break;
			case KNIGHT_BLACK:
				generateKnightDiscoveredCheckingMoves(from);
				break;
			case BISHOP_BLACK:
				generateBishopDiscoveredCheckingMoves(from);
				break;
			case ROOK_BLACK:
				generateRookDiscoveredCheckingMoves(from);
				break;
			case KING_BLACK:
				generateKingDiscoveredCheckingMoves(from);
				break;
			// Queen cannot open discovered check, therefore is not considered here
			default:
				assert(false);
		}
		discPiecePos &= (discPiecePos - 1);
	}
}

// Generates all pawn discovered checking moves from square from
void MoveGenerator::generatePawnDiscoveredCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay() && from < A7 && (from % 8 != _positionState->blackKingPosition() % 8)) {
		// Promotion moves are not considered here, because they were already considered in generatePawnWhiteCapturingMoves
		// Last condition ensures that pawn and opponent king are not on the same file, in which case pawn
		// cannot open discovered check
		Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares());
		// Direct checking moves are not removed from moveBoard, because discovered pawn cannot move
		// to the position to give direct check
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (to - from == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
	else if (!_positionState->whiteToPlay() && from > H2 && (from % 8 != _positionState->whiteKingPosition() % 8)) {
		// Promotion moves are not considered here, because they were already considered in generatePawnBlackCapturingMoves
		// Last condition ensures that pawn and opponent king are not on the same file, in which case pawn
		// cannot open discovered check
		Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares());
		// Direct checking moves are not removed from moveBoard, because discovered pawn cannot move
		// to the position to give direct check
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (from - to == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all knight discovered checking moves from square from
void MoveGenerator::generateKnightDiscoveredCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_WHITE];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[KNIGHT_BLACK];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all rook discovered checking moves from square from
void MoveGenerator::generateRookDiscoveredCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_WHITE];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[ROOK_BLACK];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all bishop discovered checking moves from square from
void MoveGenerator::generateBishopDiscoveredCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_WHITE];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares() & ~_positionState->getDirectCheck()[BISHOP_BLACK];
		// Direct checking moves are removed from moveBoard, because they have been already
		// generated in generateDirectCheckingMoves
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all king discovered checking moves from square from
void MoveGenerator::generateKingDiscoveredCheckingMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) &
			~(RankFileMask[_positionState->blackKingPosition()] | DiagonalMask[_positionState->blackKingPosition()]) &
			// Moves of the white king which are on the ray coming from black king
			// position cannot open discovered check	
		   	~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->blackKingPosition());
	   while (moveBoard) {
		   Square to = (Square) _bitboardImpl->lsb(moveBoard);
		   (_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		   moveBoard &= (moveBoard - 1);
	   }
	}
	else {
		Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) &
			~(RankFileMask[_positionState->whiteKingPosition()] | DiagonalMask[_positionState->whiteKingPosition()]) &
			// Moves of black king which are on the ray coming from white king
			// position cannot open discovered check	
		   	~_positionState->occupiedSquares() & ~_bitboardImpl->kingAttackFrom(_positionState->whiteKingPosition());
	   while (moveBoard) {
		   Square to = (Square) _bitboardImpl->lsb(moveBoard);
		   (_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		   moveBoard &= (moveBoard - 1);
	   }
	}
}

// Generates all quite moves, that is the moves which
// are non-capturing, non-promotion moves
void MoveGenerator::generateQuiteMoves()
{
	if (_positionState->whiteToPlay()) {
		Bitboard pawnsPos = _positionState->getPiecePos()[PAWN_WHITE];
		while (pawnsPos) {
			Square from = (Square) _bitboardImpl->lsb(pawnsPos);
			generatePawnWhiteQuiteMoves(from);
			pawnsPos &= (pawnsPos - 1);
		}

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_WHITE];
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightQuiteMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_WHITE];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookQuiteMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_WHITE];
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

		Bitboard knightsPos = _positionState->getPiecePos()[KNIGHT_BLACK];
		while (knightsPos) {
			Square from = (Square) _bitboardImpl->lsb(knightsPos);
			generateKnightQuiteMoves(from);
			knightsPos &= (knightsPos - 1);
		}

		Bitboard rooksPos = _positionState->getPiecePos()[ROOK_BLACK];
		while (rooksPos) {
			Square from = (Square) _bitboardImpl->lsb(rooksPos);
			generateRookQuiteMoves(from);
			rooksPos &= (rooksPos - 1);
		}

		Bitboard bishopsPos = _positionState->getPiecePos()[BISHOP_BLACK];
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

// Generates all pawn white quite moves from square from
// that is all non-promotion, non-capturing moves
void MoveGenerator::generatePawnWhiteQuiteMoves(Square from)
{
	if (from < A7) {
		// Promotions are not considered here, becuase they were generated in generateCapturingMoves
		Bitboard moveBoard = _bitboardImpl->pawnWhiteMovesFrom(from, _positionState->occupiedSquares());
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (to - from == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all pawn black quite moves from square from
// that is all non-promotion, non-capturing moves
void MoveGenerator::generatePawnBlackQuiteMoves(Square from)
{
	if (from > H2) {
		// Promotions are not considered here, becuase they were generated in generateCapturingMoves
		Bitboard moveBoard = _bitboardImpl->pawnBlackMovesFrom(from, _positionState->occupiedSquares());
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			if (from - to == 16) {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, EN_PASSANT_MOVE);
			}
			else {
				(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			}
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all knight quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateKnightQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->knightAttackFrom(from) & ~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all rook quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateRookQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->rookAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all bishop quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateBishopQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->bishopAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all queen quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateQueenQuiteMoves(Square from)
{
	if (_positionState->whiteToPlay()) {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
	else {
		Bitboard moveBoard = _bitboardImpl->queenAttackFrom(from, _positionState->occupiedSquares()) &
		   	~_positionState->occupiedSquares();
		while (moveBoard) {
			Square to = (Square) _bitboardImpl->lsb(moveBoard);
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
			moveBoard &= (moveBoard - 1);
		}
	}
}

// Generates all white king quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateKingWhiteQuiteMoves(Square from)
{
	Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_positionState->occupiedSquares() &
	   	~_bitboardImpl->kingAttackFrom(_positionState->blackKingPosition());
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}

	if (from == E1) {
		if (_positionState->whiteLeftCastling() && !(WHITE_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, C1, ETY_SQUARE, CASTLING_MOVE);
		}
		if (_positionState->whiteRightCastling() && !(WHITE_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, G1, ETY_SQUARE, CASTLING_MOVE);
		}
	}
}

// Generates all black king quite moves from square from
// that is all non-capturing moves
void MoveGenerator::generateKingBlackQuiteMoves(Square from)
{
	Bitboard moveBoard = _bitboardImpl->kingAttackFrom(from) & ~_positionState->occupiedSquares() &
	   	~_bitboardImpl->kingAttackFrom(_positionState->whiteKingPosition());
	while (moveBoard) {
		Square to = (Square) _bitboardImpl->lsb(moveBoard);
		(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, to, ETY_SQUARE, NORMAL_MOVE);
		moveBoard &= (moveBoard - 1);
	}

	if (from == E8) {
		if (_positionState->blackLeftCastling() && !(BLACK_LEFT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, C8, ETY_SQUARE, CASTLING_MOVE);
		}
		if (_positionState->blackRightCastling() && !(BLACK_RIGHT_CASTLING_ETY_SQUARES & _positionState->occupiedSquares())) {
			(_moveGenInfo->_availableMoves)[(_moveGenInfo->_availableMovesSize)++] = MoveInfo(from, G8, ETY_SQUARE, CASTLING_MOVE);
		}
	}
}

void MoveGenerator::sortCapturingMoves()
{
}

// Evaluates the Static Exchange Evaluation using swap algorithm
// for the move; return true if move results in eventual gain for the
// side
int MoveGenerator::SEE(const MoveInfo& move)
{
	uint16_t depth = 0;
	bool whiteToPlay = _positionState->whiteToPlay();
	Bitboard occupiedSquares = _positionState->occupiedSquares();
	Bitboard movedPieces = 0;
	Bitboard attackingPiecePos = squareToBitboard[move.from];
	Piece attackingPiece = _positionState->getBoard()[mRank(move.from)][mFile(move.from)];
	_gainSEE[depth] = -PIECE_VALUES[_positionState->getBoard()[mRank(move.to)][mFile(move.to)]];
	while (attackingPiecePos && depth < MAX_SEARCH_DEPTH) {
		++depth;
		whiteToPlay = !whiteToPlay;
		_gainSEE[depth] = -PIECE_VALUES[attackingPiece] + _gainSEE[depth - 1];
		if (_gainSEE[depth - 1] * _gainSEE[depth] > 0) {
			break;
		}
		occupiedSquares ^= attackingPiecePos;
		movedPieces |= attackingPiecePos;
		attackingPiecePos = getLeastValuablePiece(move.to, whiteToPlay, movedPieces, occupiedSquares,  attackingPiece);
	}
	while (--depth) {
		whiteToPlay = !whiteToPlay;
		if (whiteToPlay) {
			_gainSEE[depth - 1] = std::max(_gainSEE[depth - 1], _gainSEE[depth]);
		}
		else {
			_gainSEE[depth - 1] = std::min(_gainSEE[depth - 1], _gainSEE[depth]);
		}
	}
	if (whiteToPlay) {
		return -_gainSEE[0];
	}
	else {
		return _gainSEE[0];
	}
}
// Returns the bitboard of the position of the least valuable 
// peace which attacks the square to, when several pieces has been
// moved by SEE algorithm; sets attackingPiece to the attacking piece
// value
Bitboard MoveGenerator::getLeastValuablePiece(Square to, bool whiteToPlay, const Bitboard& movedPieces, const Bitboard& occupiedSquares, Piece& attackingPiece) const
{
	Bitboard attackingPos = 0;
	if (whiteToPlay) {
		attackingPos = _bitboardImpl->pawnsWhiteAttackTo(to, _positionState->getPiecePos()[PAWN_WHITE]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = PAWN_WHITE;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->knightsAttackTo(to, _positionState->getPiecePos()[KNIGHT_WHITE]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = KNIGHT_WHITE;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->bishopsAttackTo(to, occupiedSquares, _positionState->getPiecePos()[BISHOP_WHITE]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = BISHOP_WHITE;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->rooksAttackTo(to, occupiedSquares, _positionState->getPiecePos()[ROOK_WHITE]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = ROOK_WHITE;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->bishopsAttackTo(to, occupiedSquares, _positionState->getPiecePos()[QUEEN_WHITE]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = QUEEN_WHITE;
			return attackingPos & -attackingPos;
		}
	}
	else {
		attackingPos = _bitboardImpl->pawnsWhiteAttackTo(to, _positionState->getPiecePos()[PAWN_BLACK]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = PAWN_BLACK;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->knightsAttackTo(to, _positionState->getPiecePos()[KNIGHT_BLACK]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = KNIGHT_BLACK;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->bishopsAttackTo(to, occupiedSquares, _positionState->getPiecePos()[BISHOP_BLACK]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = BISHOP_BLACK;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->rooksAttackTo(to, occupiedSquares, _positionState->getPiecePos()[ROOK_BLACK]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = ROOK_BLACK;
			return attackingPos & -attackingPos;
		}
		attackingPos = _bitboardImpl->bishopsAttackTo(to, occupiedSquares, _positionState->getPiecePos()[QUEEN_BLACK]) & ~movedPieces;
		if (attackingPos) {
			attackingPiece = QUEEN_BLACK;
			return attackingPos & -attackingPos;
		}
	}
	attackingPiece = ETY_SQUARE;
	return 0;
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

MoveGenerator::~MoveGenerator()
{
	delete _gainSEE;
}

}
