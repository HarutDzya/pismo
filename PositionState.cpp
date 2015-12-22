#include "PositionState.h"
#include "BitboardImpl.h"
#include "ZobKeyImpl.h"
#include "PieceSquareTable.h"
#include <assert.h>
#include <cstdlib>
#include <iostream>

namespace pismo
{

PositionState::PositionState(): 
_whitePieces(0), 
_blackPieces(0),
_occupiedSquares(0),
_discPiecePos(0),
_pinPiecePos(0),
_bitboardImpl(BitboardImpl::instance()),
_zobKeyImpl(new ZobKeyImpl()),
_absolutePinsPos(0),
_isDoubleCheck(false),
_whiteToPlay(true),
_enPassantFile(-1),
_whiteKingPosition(E1),
_blackKingPosition(E8),
_kingUnderCheck(false),
_whiteLeftCastling(false),
_whiteRightCastling(false),
_blackLeftCastling(false),
_blackRightCastling(false),
_isMiddleGame(true),
_zobKey(0),
_materialZobKey(0),
_pstValue(0),
_moveStack(),
_halfmoveClock(0),
_fullmoveCount(1)
{
	for (unsigned int i = 0; i < 8; ++i) {
		for (unsigned int j = 0; j < 8; ++j) {
			_board[i][j] = ETY_SQUARE;
		}
	}
	for (unsigned int i = 0; i < PIECE_NB; ++i) {
		_piecePos[i] = 0;
		_pieceCount[i] = 0;
	} 
	
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
		_materialZobKey ^= _zobKeyImpl->getMaterialKey((Piece)(piece), 0);
	}
}

PositionState::~PositionState()
{
	delete _zobKeyImpl;
}

void PositionState::setPiece(Square s, Piece p)
{
	_board[s / 8][s % 8] = p;
	if (p <= KING_WHITE) {
		if (p == KING_WHITE) {
			_whiteKingPosition = s;
		}
		addPieceToBitboards<WHITE>(s, p);
	}
	else {
		if (p == KING_BLACK) {
			_blackKingPosition = s;
		}
		addPieceToBitboards<BLACK>(s, p);
	}
	++_pieceCount[p];
	_pstValue += calculatePstValue(p, s);
	_zobKey ^=  _zobKeyImpl->getPieceAtSquareKey(p, s);

	_materialZobKey ^= _zobKeyImpl->getMaterialKey(p, _pieceCount[p]);
}

void PositionState::initPosition(const std::vector<std::pair<Square, Piece> >& pieces)
{
	if (initPositionIsValid(pieces)) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			setPiece(pieces[i].first, pieces[i].second);
		}
	
		_whiteLeftCastling = true;
		_whiteRightCastling = true;
		_blackLeftCastling = true;
		_blackRightCastling = true;
		_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
		_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
		_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
		_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();

		_occupiedSquares = _whitePieces | _blackPieces;
		updateCheckStatus();
		updateGameStatus();
	}
}
/* Checks for the following conditions for initial position validty:
	1) That there are no pawns on the first and last ranks
	2) For each square there is at most one entry
	3) Number of pieces for each color is not more than 16	
	4) Number of pawns for each color is not more than 8
	5) Each color has only one king
	6) Number of promoted pieces for each color is not more than the missing pawns 
*/
bool PositionState::initPositionIsValid(const std::vector<std::pair<Square, Piece> >& pieces) const
{
	Piece board[NUMBER_OF_SQUARES];
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		board[sq] = ETY_SQUARE;
	} 
	unsigned int piecesCount[PIECE_NB] = {};
	unsigned int whitePiecesSum = 0;
	unsigned int blackPiecesSum = 0;
	for (std::size_t i = 0; i < pieces.size(); ++i) {
		assert(pieces[i].first >= A1 && pieces[i].first <= H8);
		assert(pieces[i].second >= PAWN_WHITE && pieces[i].second <= KING_BLACK);
		if ((pieces[i].second == PAWN_WHITE || pieces[i].second == PAWN_BLACK) &&
		(pieces[i].first <= H1 || pieces[i].first >= A8)) {
			return false;
		}
		else {
			if (pieces[i].second <= KING_WHITE) {
				++piecesCount[pieces[i].second];
				++whitePiecesSum;
			}
			else {
				++piecesCount[pieces[i].second];
				++blackPiecesSum;
			}
		}
		
		if (board[pieces[i].first] != ETY_SQUARE) {
			return false;
		}
		else {
			board[pieces[i].first] = pieces[i].second;
		}
	}
	
	if (whitePiecesSum > 16 || blackPiecesSum > 16) {
		return false;
	}

	unsigned int whitePromotedPiecesSum = 0;
	unsigned int blackPromotedPiecesSum = 0;

	for (unsigned int piece = PAWN_WHITE; piece <= KING_BLACK; ++piece) {
		switch(piece) {
			case PAWN_WHITE: 
				if (piecesCount[piece] > 8) {
					return false;
				}
				break;
			case PAWN_BLACK:
				if (piecesCount[piece] > 8) {
					return false;
				}
				break;
			case KING_WHITE:
				if (piecesCount[piece] != 1) {
					return false;
				}
				break;
			case KING_BLACK:
				if (piecesCount[piece] != 1) {
					return false;
				}
				break;
			case QUEEN_WHITE:
				whitePromotedPiecesSum += ((int) piecesCount[piece] - 1) > 0 ? piecesCount[piece] - 1 : 0;
				break;
			case QUEEN_BLACK:
				blackPromotedPiecesSum += ((int) piecesCount[piece] - 1) > 0 ? piecesCount[piece] - 1 : 0;
				break;
			default:
				if (piece < KING_WHITE) {
					whitePromotedPiecesSum += ((int) piecesCount[piece] - 2) > 0 ? piecesCount[piece] - 2 : 0;
				}
				else {
				 	blackPromotedPiecesSum += ((int) piecesCount[piece] - 2) > 0 ? piecesCount[piece] - 2 : 0;
				}
		}
	}
	
	if ((whitePromotedPiecesSum > 8 - piecesCount[PAWN_WHITE]) || (blackPromotedPiecesSum > 8 - piecesCount[PAWN_BLACK])) {
		return false;
	}
	
	return true; 
}

void PositionState::initPositionFEN(const std::string& fen)
{
	unsigned int charCount = 0;
	initMaterialFEN(fen, charCount);
	++charCount;
	initRightToPlayFEN(fen, charCount);
	++charCount;
	initCastlingRightsFEN(fen, charCount);
	++charCount;
	initEnPassantFileFEN(fen, charCount);
	++charCount;
	initMoveCountFEN(fen, charCount);

	_occupiedSquares = _whitePieces | _blackPieces;	
	updateCheckStatus();
	updateGameStatus();
	assert(charCount == fen.size());
}


void PositionState::initMaterialFEN(const std::string& fen, unsigned int& charCount)
{
	std::vector<std::pair<Square, Piece> > pieces;
	unsigned int rank = 7;
	unsigned int file = 0;
	while(fen[charCount] != ' ') {
		switch(fen[charCount]) {
			case 'P':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), PAWN_WHITE));
				++file;
				break;
			case 'N':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KNIGHT_WHITE));
				++file;
				break;
			case 'B':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), BISHOP_WHITE));
				++file;
				break;
			case 'R':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), ROOK_WHITE));
				++file;
				break;
			case 'Q':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), QUEEN_WHITE));
				++file;
				break;
			case 'K':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KING_WHITE));
				++file;
				break;
			case 'p':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), PAWN_BLACK));
				++file;
				break;
			case 'n':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KNIGHT_BLACK));
				++file;
				break;
			case 'b':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), BISHOP_BLACK));
				++file;
				break;
			case 'r':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), ROOK_BLACK));
				++file;
				break;
			case 'q':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), QUEEN_BLACK));
				++file;
				break;
			case 'k':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KING_BLACK));
				++file;
				break;
			case '1':
				file += 1;
				break;
			case '2':
				file += 2;
			 	break;
			case '3':
				file += 3;
				break;
			case '4':
				file += 4;
				break;
			case '5':
				file += 5;
				break;
			case '6':
				file += 6;
				break;
			case '7':
				file += 7;
				break;
			case '8':
				// file is updated directly in case '/'
				break;
			case '/':
				--rank;
				file = 0;
				break;
			default:
				assert(false);
		}
		
		++charCount;
	}

	if (initPositionIsValid(pieces)) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			setPiece(pieces[i].first, pieces[i].second);
		}
	}
}

void PositionState::initRightToPlayFEN(const std::string& fen, unsigned int& charCount)
{
	if(fen[charCount++] == 'w') {
		_whiteToPlay = true;
	}
	else {
		_whiteToPlay = false;
		_zobKey ^= _zobKeyImpl->getIfBlackToPlayKey();
	}
}


void PositionState::initCastlingRightsFEN(const std::string& fen, unsigned int& charCount)
{
	while(fen[charCount] != ' ') {
		switch(fen[charCount]) {
			case 'K':
				_whiteRightCastling = true;
				_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
				break;
			case 'Q':
				_whiteLeftCastling = true;
				_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
				break;
			case 'k':
				_blackRightCastling = true;
				_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();
				break;
			case 'q':
				_blackLeftCastling = true;
				_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
				break;
			case '-':
				break;
			default:
				assert(false);
		}
		++charCount;
	}
}

void PositionState::initEnPassantFileFEN(const std::string& fen, unsigned int& charCount)
{
	if(fen[charCount] == '-') {
		_enPassantFile = -1;
		++charCount;
	}
	else {	
		_enPassantFile = fen[charCount] - 'a';
		++charCount;
		assert(_enPassantFile >= 0 && _enPassantFile < 8);
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
	}
}

void PositionState::initMoveCountFEN(const std::string& fen, unsigned int& charCount)
{
	_halfmoveClock = std::atoi(&fen[charCount]);
	while(fen[charCount] != ' ') {
		++charCount;
	}
	_fullmoveCount = std::atoi(&fen[charCount]);
	while(charCount != fen.size()) {
		++charCount;
	}
}

void PositionState::updateCheckStatus()
{
	if(_whiteToPlay) {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			Square pieceSq = (Square) sq;
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_BLACK:
					if (squareToBitboard[_whiteKingPosition] & _bitboardImpl->pawnBlackAttackFrom(pieceSq)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = squareToBitboard[pieceSq];
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case KNIGHT_BLACK:
					if (squareToBitboard[_whiteKingPosition] & _bitboardImpl->knightAttackFrom(pieceSq)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = squareToBitboard[pieceSq];
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case BISHOP_BLACK:
					if (squareToBitboard[_whiteKingPosition] & _bitboardImpl->bishopAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _whiteKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case ROOK_BLACK:
					if (squareToBitboard[_whiteKingPosition] & _bitboardImpl->rookAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _whiteKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case QUEEN_BLACK:
					if (squareToBitboard[_whiteKingPosition] & _bitboardImpl->queenAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _whiteKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case KING_BLACK:	
					assert(!(squareToBitboard[_whiteKingPosition] & _bitboardImpl->kingAttackFrom(pieceSq)));
					break;
				default:
					break;
			}
		}
	}
	else {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			Square pieceSq = (Square) sq;
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_WHITE:
					if (squareToBitboard[_blackKingPosition] & _bitboardImpl->pawnWhiteAttackFrom(pieceSq)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = squareToBitboard[pieceSq];
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case KNIGHT_WHITE:
					if (squareToBitboard[_blackKingPosition] & _bitboardImpl->knightAttackFrom(pieceSq)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = squareToBitboard[pieceSq];
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case BISHOP_WHITE:
					if (squareToBitboard[_blackKingPosition] & _bitboardImpl->bishopAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _blackKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case ROOK_WHITE:
					if (squareToBitboard[_blackKingPosition] & _bitboardImpl->rookAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _blackKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case QUEEN_WHITE:
					if (squareToBitboard[_blackKingPosition] & _bitboardImpl->queenAttackFrom(pieceSq, _occupiedSquares)) {
						if (!_kingUnderCheck) {
							_kingUnderCheck = true;
							_absolutePinsPos = _bitboardImpl->getSquaresBetween(pieceSq, _blackKingPosition);
						}
						else {
							_isDoubleCheck = true;
							return;
						}
					}
					break;
				case KING_BLACK:	
					assert(!(squareToBitboard[_blackKingPosition] & _bitboardImpl->kingAttackFrom(pieceSq)));
					break;
				default:
					break;
			}
		}
	}
}

bool PositionState::pieceIsSlidingPiece(Piece piece) const
{
	if (piece == ROOK_WHITE || piece == BISHOP_WHITE || piece == QUEEN_WHITE ||
			piece == ROOK_BLACK || piece == BISHOP_BLACK || piece == QUEEN_BLACK) {
		return true;
	}
	
	return false;
}

void PositionState::updateDirectCheckArray()
{
	if (_whiteToPlay) {
		_directCheck[KING_WHITE] = 0;
		_directCheck[KNIGHT_WHITE] = _bitboardImpl->knightAttackFrom(_blackKingPosition);
		_directCheck[PAWN_WHITE] = _bitboardImpl->pawnsWhiteAttackTo(_blackKingPosition);
		_directCheck[ROOK_WHITE] = _bitboardImpl->rookAttackFrom(_blackKingPosition, _occupiedSquares);
		_directCheck[BISHOP_WHITE] = _bitboardImpl->bishopAttackFrom(_blackKingPosition, _occupiedSquares);
		_directCheck[QUEEN_WHITE] = _bitboardImpl->queenAttackFrom(_blackKingPosition, _occupiedSquares);
	}
	else {
		_directCheck[KING_BLACK] = 0;
		_directCheck[KNIGHT_BLACK] = _bitboardImpl->knightAttackFrom(_whiteKingPosition);
		_directCheck[PAWN_BLACK] = _bitboardImpl->pawnsBlackAttackTo(_whiteKingPosition);
		_directCheck[ROOK_BLACK] = _bitboardImpl->rookAttackFrom(_whiteKingPosition, _occupiedSquares);
		_directCheck[BISHOP_BLACK] = _bitboardImpl->bishopAttackFrom(_whiteKingPosition, _occupiedSquares);
		_directCheck[QUEEN_BLACK] = _bitboardImpl->queenAttackFrom(_whiteKingPosition, _occupiedSquares);

		}
}

void PositionState::updateDiscoveredChecksInfo()
{
	_discPiecePos = 0;
	if (_whiteToPlay) {
		if (DiagonalMask[_blackKingPosition] & (_piecePos[BISHOP_WHITE] | _piecePos[QUEEN_WHITE]))
		{
			Bitboard possibleDiscPieces = _bitboardImpl->bishopAttackFrom(_blackKingPosition, _occupiedSquares) &
				(_whitePieces | _piecePos[PAWN_BLACK]);
			Bitboard slidingPiecePos = _bitboardImpl->bishopAttackFrom(_blackKingPosition, _occupiedSquares ^ possibleDiscPieces) &
				(_piecePos[BISHOP_WHITE] | _piecePos[QUEEN_WHITE]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_discPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _blackKingPosition) & possibleDiscPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}

		if (RankFileMask[_blackKingPosition] & (_piecePos[ROOK_WHITE] | _piecePos[QUEEN_WHITE]))
		{
			Bitboard possibleDiscPieces = _bitboardImpl->rookAttackFrom(_blackKingPosition, _occupiedSquares) &
				(_whitePieces | _piecePos[PAWN_BLACK]);
			Bitboard slidingPiecePos = _bitboardImpl->rookAttackFrom(_blackKingPosition, _occupiedSquares ^ possibleDiscPieces) &
				(_piecePos[ROOK_WHITE] | _piecePos[QUEEN_WHITE]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_discPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _blackKingPosition) & possibleDiscPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}
	}
	else {
		if (DiagonalMask[_whiteKingPosition] & (_piecePos[BISHOP_BLACK] | _piecePos[QUEEN_BLACK]))
		{
			Bitboard possibleDiscPieces = _bitboardImpl->bishopAttackFrom(_whiteKingPosition, _occupiedSquares) &
				(_blackPieces | _piecePos[PAWN_WHITE]);
			Bitboard slidingPiecePos = _bitboardImpl->bishopAttackFrom(_whiteKingPosition, _occupiedSquares ^ possibleDiscPieces) &
				(_piecePos[BISHOP_BLACK] | _piecePos[QUEEN_BLACK]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_discPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _whiteKingPosition) & possibleDiscPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}

		if (RankFileMask[_whiteKingPosition] & (_piecePos[ROOK_BLACK] | _piecePos[QUEEN_BLACK]))
		{
			Bitboard possibleDiscPieces = _bitboardImpl->rookAttackFrom(_whiteKingPosition, _occupiedSquares) &
				(_blackPieces | _piecePos[PAWN_WHITE]);
			Bitboard slidingPiecePos = _bitboardImpl->rookAttackFrom(_whiteKingPosition, _occupiedSquares ^ possibleDiscPieces) &
				(_piecePos[ROOK_BLACK] | _piecePos[QUEEN_BLACK]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_discPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _whiteKingPosition) & possibleDiscPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}
	}
}

void  PositionState::updateMoveChecksOpponentKing(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	_absolutePinsPos = 0;
	_kingUnderCheck = false;
	_isDoubleCheck = false;
	Square kingSq = _whiteToPlay ? _blackKingPosition : _whiteKingPosition;
		
	if (squareToBitboard[move.to] & _directCheck[pfrom]) {
		if (pieceIsSlidingPiece(pfrom)) {
				_absolutePinsPos |= _bitboardImpl->getSquaresBetween(move.to, kingSq);
		}
		else {
			_absolutePinsPos |= squareToBitboard[move.to];
		}
		 _kingUnderCheck = true;
	}	   
		
	Square slidingPiecePos;	
	if (moveOpensDiscoveredCheck(move, slidingPiecePos)) {
		if (_kingUnderCheck) {
			_isDoubleCheck = true;
			return;
		}
		_absolutePinsPos |= _bitboardImpl->getSquaresBetween(slidingPiecePos, kingSq);
		_kingUnderCheck = true;
	}

	if (move.type == EN_PASSANT_CAPTURE) {
		if (enPassantCaptureDiscoveresCheck(move, slidingPiecePos)) {
			if (_kingUnderCheck) {
				_isDoubleCheck = true;
				return;
			}
			_absolutePinsPos |= _bitboardImpl->getSquaresBetween(slidingPiecePos, kingSq);
			_kingUnderCheck = true;
		}
	}	   

	if (move.promoted != ETY_SQUARE) {
		if (promotionMoveChecksOpponentKing(move)) {
			if (_kingUnderCheck) {
				_isDoubleCheck = true;
				return;
			}

			if (pieceIsSlidingPiece(move.promoted)) {
				_absolutePinsPos |= _bitboardImpl->getSquaresBetween(move.to, kingSq);
			}
			else {
				//promoted knight gives check
				_absolutePinsPos = squareToBitboard[move.to];
			}
			_kingUnderCheck = true;
		}
	}
		
	if (move.type == CASTLING_MOVE) {
		if (castlingChecksOpponentKing(move, slidingPiecePos)) {
			_kingUnderCheck = true;
			_absolutePinsPos |= _bitboardImpl->getSquaresBetween(slidingPiecePos, kingSq);
		}
	}
}

bool PositionState::moveOpensDiscoveredCheck(const MoveInfo& move, Square& slidingPiecePos) const
{
	slidingPiecePos = INVALID_SQUARE;
	if (squareToBitboard[move.from] & _discPiecePos) {
		if (_whiteToPlay) {
			if (!(_bitboardImpl->getSquaresBetween(move.from, _blackKingPosition) &
					   	_bitboardImpl->getSquaresBetween(move.to, _blackKingPosition))) {
				Bitboard slidingPieceBoard = (_bitboardImpl->queenAttackFrom(_blackKingPosition, _occupiedSquares) ^
					_bitboardImpl->queenAttackFrom(_blackKingPosition, _occupiedSquares ^ squareToBitboard[move.from])) &
					_whitePieces;
				assert (slidingPieceBoard);
				slidingPiecePos = (Square) _bitboardImpl->lsb(slidingPieceBoard);
				return true;
			}
		}
		else {
			if (!(_bitboardImpl->getSquaresBetween(move.from, _whiteKingPosition) &
					   	_bitboardImpl->getSquaresBetween(move.to, _whiteKingPosition))) {
				Bitboard slidingPieceBoard = (_bitboardImpl->queenAttackFrom(_whiteKingPosition, _occupiedSquares) ^
					_bitboardImpl->queenAttackFrom(_whiteKingPosition, _occupiedSquares ^ squareToBitboard[move.from])) &
					_blackPieces;
				assert (slidingPieceBoard);
				slidingPiecePos = (Square) _bitboardImpl->lsb(slidingPieceBoard);
				return true;
			}
		}
	}
	return false;
}

bool PositionState::castlingChecksOpponentKing(const MoveInfo& move, Square& slidingPiecePos) const
{
	if (_whiteToPlay) {
		if (_blackKingPosition / 8 != 0) {
			if (move.to == C1) {
				if (squareToBitboard[D1] & _directCheck[ROOK_WHITE]) {
					slidingPiecePos = D1;
					return true;
				}
			}
			else {
				assert (move.to == G1);
				if (squareToBitboard[F1] & _directCheck[ROOK_WHITE]) {
					slidingPiecePos = F1;
					return true;
				}
			}
		}
		else {
			if (squareToBitboard[E1] & _directCheck[ROOK_WHITE]) {
				slidingPiecePos = (move.to > move.from) ? F1 : D1;
				return true;
			}
		}
	}
	else {
		if (_whiteKingPosition / 8 != 7) {
			if (move.to == C8) {
				if (squareToBitboard[D8] & _directCheck[ROOK_BLACK]) {
					slidingPiecePos = D8;
					return true;
				}
			}
			else {
				assert (move.to == G8);
				if (squareToBitboard[F8] & _directCheck[ROOK_BLACK]) {
					slidingPiecePos = F8;
					return true;
				}
			}
		}
		else {
			if (squareToBitboard[E8] & _directCheck[ROOK_WHITE]) {
				slidingPiecePos = (move.to > move.from) ? F8: D8;
				return true;
			}
		}
	}
	slidingPiecePos = INVALID_SQUARE;
	return false;
}
			
bool PositionState::enPassantCaptureDiscoveresCheck(const MoveInfo& move, Square& slidingPiecePos) const
{
	slidingPiecePos = INVALID_SQUARE;
	if (_whiteToPlay && _blackKingPosition / 8 == 4) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _occupiedSquares, leftPos, rightPos);
		if (leftPos != INVALID_SQUARE && rightPos != INVALID_SQUARE) {
			if (leftPos == _blackKingPosition) {
				if (_board[rightPos / 8][rightPos % 8] == ROOK_WHITE || _board[rightPos / 8][rightPos % 8] == QUEEN_WHITE) {
					slidingPiecePos = rightPos;
					return true;
				}
			}
			if (rightPos == _blackKingPosition) {
				if (_board[leftPos / 8][leftPos % 8] == ROOK_WHITE || _board[leftPos / 8][leftPos % 8] == QUEEN_WHITE) {
					slidingPiecePos = leftPos;
					return true;
				}
			}
		}
	}
	else if (!_whiteToPlay && _whiteKingPosition / 8 == 3) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _occupiedSquares, leftPos, rightPos);
		if (leftPos != INVALID_SQUARE && rightPos != INVALID_SQUARE) {
			if (leftPos == _whiteKingPosition) {
				if (_board[rightPos / 8][rightPos % 8] == ROOK_BLACK || _board[rightPos / 8][rightPos % 8] == QUEEN_BLACK) {
					slidingPiecePos = rightPos;
					return true;
				}
			}
			if (rightPos == _whiteKingPosition) {
				if (_board[leftPos / 8][leftPos % 8] == ROOK_BLACK || _board[leftPos / 8][leftPos % 8] == QUEEN_BLACK) {
					slidingPiecePos = leftPos;
					return true;
				}
			}
		}
	}

	Square capturedPawnPos = (move.to > move.from) ? (Square) (move.to - 8) : (Square) (move.to + 8);
	
	if (squareToBitboard[capturedPawnPos] & _discPiecePos) {
		if (_whiteToPlay) {
			Bitboard slidingPieceBoard = (_bitboardImpl->queenAttackFrom(_blackKingPosition, _occupiedSquares) ^
				_bitboardImpl->queenAttackFrom(_blackKingPosition, _occupiedSquares ^ squareToBitboard[capturedPawnPos])) &
				_whitePieces;
			assert (slidingPieceBoard);
			slidingPiecePos = (Square) _bitboardImpl->lsb(slidingPieceBoard);
			return true;
		}
		else {
			Bitboard slidingPieceBoard = (_bitboardImpl->queenAttackFrom(_whiteKingPosition, _occupiedSquares) ^
				_bitboardImpl->queenAttackFrom(_whiteKingPosition, _occupiedSquares ^ squareToBitboard[capturedPawnPos])) &
				_blackPieces;
				assert (slidingPieceBoard);
				slidingPiecePos = (Square) _bitboardImpl->lsb(slidingPieceBoard);
				return true;
		}
		
	}
	
	return false;
}

bool PositionState::promotionMoveChecksOpponentKing(const MoveInfo& move) const
{
	if (squareToBitboard[move.to] & _directCheck[move.promoted]) {
			return true;
	}

	if (pieceIsSlidingPiece(move.promoted)) {
		Square kingSq = _whiteToPlay ? _blackKingPosition : _whiteKingPosition;
		if ((squareToBitboard[move.from] & _directCheck[move.promoted]) &&
				squareToBitboard[move.from] & _bitboardImpl->getSquaresBetween(move.to, kingSq)) {
			return true;
		}
	}

	return false;
}

void PositionState::updateStatePinInfo()
{
	_pinPiecePos = 0;
	if (_whiteToPlay) 
  {
		if (DiagonalMask[_whiteKingPosition] & (_piecePos[BISHOP_BLACK] | _piecePos[QUEEN_BLACK]))
		{
			Bitboard possiblePinPieces = _bitboardImpl->bishopAttackFrom(_whiteKingPosition, _occupiedSquares) &
				 _whitePieces;
			Bitboard slidingPiecePos = _bitboardImpl->bishopAttackFrom(_whiteKingPosition, _occupiedSquares ^ possiblePinPieces) &
			  (_piecePos[BISHOP_BLACK] | _piecePos[QUEEN_BLACK]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_pinPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _whiteKingPosition) & possiblePinPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}

		if (RankFileMask[_whiteKingPosition] & (_piecePos[ROOK_BLACK] | _piecePos[QUEEN_BLACK]))
		{
			Bitboard possiblePinPieces = _bitboardImpl->rookAttackFrom(_whiteKingPosition, _occupiedSquares) &
				_whitePieces;
			Bitboard slidingPiecePos = _bitboardImpl->rookAttackFrom(_whiteKingPosition, _occupiedSquares ^ possiblePinPieces) &
				(_piecePos[ROOK_BLACK] | _piecePos[QUEEN_BLACK]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_pinPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _whiteKingPosition) & possiblePinPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}
	}
	else {
		if (DiagonalMask[_blackKingPosition] & (_piecePos[BISHOP_WHITE] | _piecePos[QUEEN_WHITE]))
		{
			Bitboard possiblePinPieces = _bitboardImpl->bishopAttackFrom(_blackKingPosition, _occupiedSquares) &
				_blackPieces;
			Bitboard slidingPiecePos = _bitboardImpl->bishopAttackFrom(_blackKingPosition, _occupiedSquares ^ possiblePinPieces) &
				(_piecePos[BISHOP_WHITE] | _piecePos[QUEEN_WHITE]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_pinPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _blackKingPosition) & possiblePinPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}

		if (RankFileMask[_blackKingPosition] & (_piecePos[ROOK_WHITE] | _piecePos[QUEEN_WHITE]))
		{
			Bitboard possiblePinPieces = _bitboardImpl->rookAttackFrom(_blackKingPosition, _occupiedSquares) &
			_blackPieces;
			Bitboard slidingPiecePos = _bitboardImpl->rookAttackFrom(_blackKingPosition, _occupiedSquares ^ possiblePinPieces) &
				(_piecePos[ROOK_WHITE] | _piecePos[QUEEN_WHITE]);
			while (slidingPiecePos) {
				Square slidingSq = (Square) _bitboardImpl->lsb(slidingPiecePos);
				_pinPiecePos |= _bitboardImpl->getSquaresBetween(slidingSq, _blackKingPosition) & possiblePinPieces;
				slidingPiecePos &= (slidingPiecePos - 1);
			}
		}
	}
}

bool PositionState::isInterposeMove(const MoveInfo& move) const
{
	if (squareToBitboard[move.to] & _absolutePinsPos) {
		return true;
	}

	if (move.type == EN_PASSANT_CAPTURE) {
		Square capturedPiecePos = _whiteToPlay ? (Square) (move.to - 8) : (Square) (move.to + 8);
		if (squareToBitboard[capturedPiecePos] & _absolutePinsPos) {
			return true;
		}
	}

	return false;
}

bool PositionState::pseudoMoveIsLegalMove(const MoveInfo& move) const
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	
	if (pfrom == KING_WHITE || pfrom == KING_BLACK) {
		return kingPseudoMoveIsLegal(move);
	}

	if ( _kingUnderCheck && (_isDoubleCheck || !isInterposeMove(move))) {
		return false;
	}

	if (pinMoveOpensCheck(move)) {
	   return false;
	}

	if ((move.type == EN_PASSANT_CAPTURE) && pinEnPassantCaptureOpensCheck(move)) {
		return false;
	}

	return true;
}

bool PositionState::kingPseudoMoveIsLegal(const MoveInfo& move) const
{
	if (move.type == CASTLING_MOVE) {
		if(!_kingUnderCheck) {
			switch (move.to) {
				case C1:
					return !squareUnderAttack(D1) && !squareUnderAttack(C1);
					break;
				case G1:
					return !squareUnderAttack(F1) && !squareUnderAttack(G1);
					break;
				case C8:
					return !squareUnderAttack(D8) && !squareUnderAttack(C8);
					break;
				case G8:
					return !squareUnderAttack(F8) && !squareUnderAttack(G8);
					break;
				default:
					assert(false);
			}
		}

		return false;
	}

	return !squareUnderAttack(move.to);
}

bool PositionState::squareUnderAttack(Square s) const
{
	if (_whiteToPlay) {
		Bitboard occupSquares = _occupiedSquares ^ squareToBitboard[_whiteKingPosition];
		if (_bitboardImpl->pawnsBlackAttackTo(s, _piecePos[PAWN_BLACK]) || 
				_bitboardImpl->knightsAttackTo(s, _piecePos[KNIGHT_BLACK]) ||
			   	_bitboardImpl->bishopsAttackTo(s, occupSquares, _piecePos[BISHOP_BLACK]) ||
				_bitboardImpl->rooksAttackTo(s, occupSquares, _piecePos[ROOK_BLACK]) ||
				_bitboardImpl->queensAttackTo(s, occupSquares, _piecePos[QUEEN_BLACK]) ||
				_bitboardImpl->kingAttackTo(s, _piecePos[KING_BLACK])) {
			return true;
		}
	}
	else {
		Bitboard occupSquares = _occupiedSquares ^ squareToBitboard[_blackKingPosition];
		if (_bitboardImpl->pawnsWhiteAttackTo(s, _piecePos[PAWN_WHITE]) || 
				_bitboardImpl->knightsAttackTo(s, _piecePos[KNIGHT_WHITE]) ||
			   	_bitboardImpl->bishopsAttackTo(s, occupSquares, _piecePos[BISHOP_WHITE]) ||
				_bitboardImpl->rooksAttackTo(s, occupSquares, _piecePos[ROOK_WHITE]) ||
				_bitboardImpl->queensAttackTo(s, occupSquares, _piecePos[QUEEN_WHITE]) ||
				_bitboardImpl->kingAttackTo(s, _piecePos[KING_WHITE])) {
			return true;
		}
	}

	return false;
}


bool PositionState::pinMoveOpensCheck(const MoveInfo& move) const
{
	if (squareToBitboard[move.from] & _pinPiecePos) {
		if (_whiteToPlay) {
			if (!(_bitboardImpl->getSquaresBetween(move.from, _whiteKingPosition) &
						_bitboardImpl->getSquaresBetween(move.to, _whiteKingPosition))) {
				return true;
			}
		}
		else {
			if (!(_bitboardImpl->getSquaresBetween(move.from, _blackKingPosition) &
						_bitboardImpl->getSquaresBetween(move.to, _blackKingPosition))) {
				return true;
			}
		}
	}
	return false;
}

bool PositionState::pinEnPassantCaptureOpensCheck(const MoveInfo& move) const
{
	if (_whiteToPlay && _whiteKingPosition / 8 == 4) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _occupiedSquares, leftPos, rightPos);
		if (leftPos != INVALID_SQUARE && rightPos != INVALID_SQUARE) {
			if (leftPos == _whiteKingPosition) {
				if (_board[rightPos / 8][rightPos % 8] == ROOK_BLACK || _board[rightPos / 8][rightPos % 8] == QUEEN_BLACK) {
					return true;
				}
			}
			if (rightPos == _whiteKingPosition) {
				if (_board[leftPos / 8][leftPos % 8] == ROOK_BLACK || _board[leftPos / 8][leftPos % 8] == QUEEN_BLACK) {
					return true;
				}
			}
		}
	}
	else if (!_whiteToPlay && _blackKingPosition / 8 == 3) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _occupiedSquares, leftPos, rightPos);
		if (leftPos != INVALID_SQUARE && rightPos != INVALID_SQUARE) {
			if (leftPos == _blackKingPosition) {
				if (_board[rightPos / 8][rightPos % 8] == ROOK_WHITE || _board[rightPos / 8][rightPos % 8] == QUEEN_WHITE) {
					return true;
				}
			}
			if (rightPos == _blackKingPosition) {
				if (_board[leftPos / 8][leftPos % 8] == ROOK_WHITE || _board[leftPos / 8][leftPos % 8] == QUEEN_WHITE) {
					return true;
				}
			}
		}
	}

	return false;
}

void PositionState::makeMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	UndoMoveInfo* undoMove = _moveStack.getNextItem();
	undoMove->from = move.from;
	undoMove->to = move.to;
	undoMove->movedPiece = pfrom;
	undoMove->capturedPiece = pto;
	undoMove->enPassantFile = _enPassantFile;
	undoMove->whiteLeftCastling = _whiteLeftCastling;
	undoMove->whiteRightCastling = _whiteRightCastling;
	undoMove->blackLeftCastling = _blackLeftCastling;
	undoMove->blackRightCastling = _blackRightCastling;
	undoMove->isDoubleCheck = _isDoubleCheck;
	undoMove->absolutePinsPos = _absolutePinsPos;
	undoMove->moveType = move.type;

	updateMoveChecksOpponentKing(move);

	switch (move.type) {
		case NORMAL_MOVE:
			makeNormalMove(move);
			break;
		case CAPTURE_MOVE:
			makeCaptureMove(move);
			break;
		case PROMOTION_MOVE:
			makePromotionMove(move);
			break;
		case EN_PASSANT_MOVE:
			makeEnPassantMove(move);
			break;
		case EN_PASSANT_CAPTURE:
			makeEnPassantCapture(move);
			break;
		case CASTLING_MOVE:
			makeCastlingMove(move);
			break;
		default:
			assert(false);
	}

	updateCastlingRights(move);
	updateGameStatus();

	_occupiedSquares = _whitePieces | _blackPieces;	
	_whiteToPlay = !_whiteToPlay;
	_zobKey ^= _zobKeyImpl->getIfBlackToPlayKey();
}

void PositionState::makeNormalMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_whiteToPlay) {			
		removePieceFromBitboards<WHITE>(move.from, pfrom);
		addPieceToBitboards<WHITE>(move.to, pfrom);
		if (pfrom == KING_WHITE) {
			_whiteKingPosition = move.to;
		}
	}
	else {
		removePieceFromBitboards<BLACK>(move.from, pfrom);
		addPieceToBitboards<BLACK>(move.to, pfrom);
		if (pfrom == KING_BLACK) {
			_blackKingPosition = move.to;
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pstValue -= calculatePstValue(pfrom, move.from);
	_pstValue += calculatePstValue(pfrom, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.to);
	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
		_enPassantFile = -1;				
	}	
}

void PositionState::makeCaptureMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (_whiteToPlay) {			
		removePieceFromBitboards<WHITE>(move.from, pfrom);
		addPieceToBitboards<WHITE>(move.to, pfrom);
		removePieceFromBitboards<BLACK>(move.to, pto);
		if (pfrom == KING_WHITE) {
			_whiteKingPosition = move.to;
		}
	}
	else {
		removePieceFromBitboards<BLACK>(move.from, pfrom);
		addPieceToBitboards<BLACK>(move.to, pfrom);
		removePieceFromBitboards<WHITE>(move.to, pto);
		if (pfrom == KING_BLACK) {
			_blackKingPosition = move.to;
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pstValue -= calculatePstValue(pfrom, move.from);
	_pstValue += calculatePstValue(pfrom, move.to);
	_pstValue -= calculatePstValue(pto, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
	--_pieceCount[pto];
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
		_enPassantFile = -1;				
	}	
}

//Castling is assumed to be King's move
void PositionState::makeCastlingMove(const MoveInfo& move)
{
	if (_whiteToPlay) {
		assert(move.from == E1);
		removePieceFromBitboards<WHITE>(move.from, KING_WHITE);
		addPieceToBitboards<WHITE>(move.to, KING_WHITE);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_WHITE;
		_pstValue -= calculatePstValue(KING_WHITE, move.from);
		_pstValue += calculatePstValue(KING_WHITE, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.to);
		_whiteKingPosition = move.to;
		if (move.to == C1) {
			removePieceFromBitboards<WHITE>(A1, ROOK_WHITE);
			addPieceToBitboards<WHITE>(D1, ROOK_WHITE);
			_board[A1 / 8][A1 % 8] = ETY_SQUARE;
			_board[D1 / 8][D1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, A1);
			_pstValue += calculatePstValue(ROOK_WHITE, D1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, A1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, D1);
		}	
		else {
			assert(move.to == G1);
			removePieceFromBitboards<WHITE>(H1, ROOK_WHITE);
			addPieceToBitboards<WHITE>(F1, ROOK_WHITE);
			_board[H1 / 8][H1 % 8] = ETY_SQUARE;
			_board[F1 / 8][F1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, H1);
			_pstValue += calculatePstValue(ROOK_WHITE, F1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, H1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, F1);

		}
	}
	else {
		assert(move.from == E8);
		removePieceFromBitboards<BLACK>(move.from, KING_BLACK);
		addPieceToBitboards<BLACK>(move.to, KING_BLACK);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_BLACK;
		_pstValue -= calculatePstValue(KING_BLACK, move.from);
		_pstValue += calculatePstValue(KING_BLACK, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.to);
		_blackKingPosition = move.to;
		if (move.to == C8) {
			removePieceFromBitboards<BLACK>(A8, ROOK_BLACK);
			addPieceToBitboards<BLACK>(D8, ROOK_BLACK);
			_board[A8 / 8][A8 % 8] = ETY_SQUARE;
			_board[D8 / 8][D8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, A8);
			_pstValue += calculatePstValue(ROOK_BLACK, D8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, A8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, D8);
		}	
		else {
			assert(move.to == G8);
			removePieceFromBitboards<BLACK>(H8, ROOK_BLACK);
			addPieceToBitboards<BLACK>(F8, ROOK_BLACK);
			_board[H8 / 8][H8 % 8] = ETY_SQUARE;
			_board[F8 / 8][F8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, H8);
			_pstValue += calculatePstValue(ROOK_BLACK, F8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, H8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, F8);
		}
	}
	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
		_enPassantFile = -1;	
	}	
}

void PositionState::makeEnPassantMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_whiteToPlay) {
		removePieceFromBitboards<WHITE>(move.from, pfrom);
		addPieceToBitboards<WHITE>(move.to, pfrom);
	}
	else {
		removePieceFromBitboards<BLACK>(move.from, pfrom);
		addPieceToBitboards<BLACK>(move.to, pfrom);
	}	
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pstValue -= calculatePstValue(pfrom, move.from);
	_pstValue += calculatePstValue(pfrom, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.to);
	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
	}
	
	_enPassantFile = move.from % 8;
	_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
}

void PositionState::makeEnPassantCapture(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_whiteToPlay) {
		removePieceFromBitboards<WHITE>(move.from, pfrom);
		addPieceToBitboards<WHITE>(move.to, pfrom);
		removePieceFromBitboards<BLACK>((Square) (move.to - 8), PAWN_BLACK);
		--_pieceCount[PAWN_BLACK];
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
		_pstValue -= calculatePstValue(PAWN_BLACK, (Square) (move.to - 8));
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(PAWN_BLACK, (Square) (move.to - 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_BLACK, _pieceCount[PAWN_BLACK] + 1);
	}
	else {
		removePieceFromBitboards<BLACK>(move.from, pfrom);
		addPieceToBitboards<BLACK>(move.to, pfrom);
		removePieceFromBitboards<WHITE>((Square) (move.to + 8), PAWN_WHITE);
		--_pieceCount[PAWN_WHITE];
		_board[move.to / 8 + 1][move.to % 8] = ETY_SQUARE;
		_pstValue -= calculatePstValue(PAWN_WHITE, (Square) (move.to + 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_WHITE, _pieceCount[PAWN_WHITE] + 1);
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pstValue -= calculatePstValue(pfrom, move.from);
	_pstValue += calculatePstValue(pfrom, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.to);
	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
		_enPassantFile = -1;				
	}
}

void PositionState::makePromotionMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (_whiteToPlay) {
		assert(move.from >= A7 && move.from <= H7);
		removePieceFromBitboards<WHITE>(move.from, pfrom);
		addPieceToBitboards<WHITE>(move.to, move.promoted);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards<BLACK>(move.to, pto);
			_pstValue -= calculatePstValue(pto, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
			--_pieceCount[pto];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
		}
	}
	else {
		assert(move.from >= A2 && move.from <= H2);
		removePieceFromBitboards<BLACK>(move.from, pfrom);
		addPieceToBitboards<BLACK>(move.to, move.promoted);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards<WHITE>(move.to, pto);
			_pstValue -= calculatePstValue(pto, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
			--_pieceCount[pto];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = move.promoted;
	--_pieceCount[pfrom];
	++_pieceCount[move.promoted];
	_pstValue -= calculatePstValue(pfrom, move.from);
	_pstValue += calculatePstValue(move.promoted, move.to);

	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pfrom, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.promoted, move.to);
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(pfrom, _pieceCount[pfrom] + 1);
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.promoted, _pieceCount[move.promoted]);

	if (_enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
		_enPassantFile = -1;
	}
}

// Updates castling variables by checking whether king and rooks 
// are their designated positions after the move
void PositionState::updateCastlingRights(const MoveInfo& move)
{
	if (_whiteToPlay) {
		if (move.from == E1) {
			if (_whiteLeftCastling) {
				_whiteLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
			}
			if (_whiteRightCastling) {
				_whiteRightCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
			}
		}
		else {
			if (_whiteLeftCastling && move.from == A1) {
				_whiteLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
			}
			else if (_whiteRightCastling && move.from == H1) {
				_whiteRightCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
			}
			
			if (_blackLeftCastling && move.to == A8) {
				_blackLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
			}
			else if (_blackRightCastling && move.to == H8) {
				_blackRightCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();
			}
		}
	}
	else {
		if (move.from == E8) {
			if (_blackLeftCastling) {
				_blackLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
			}
			if (_blackRightCastling) {
				_blackRightCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();
			}
		}
		else {
			if (_blackLeftCastling && move.from == A8) {
				_blackLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
			}
			else if (_blackRightCastling && move.from == H8) {
				_blackRightCastling = false;
				_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();
			}
			
			if (_whiteLeftCastling && move.to == A1) {
				_whiteLeftCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
			}
			else if (_whiteRightCastling && move.to == H1) {
				_whiteRightCastling = false;
				_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
			}
		}
	}
}



// Adds a piece into all 4 occupation bitboards in the appropriate position
template <Color clr>
void PositionState::addPieceToBitboards(Square sq, Piece p)
{
	if (clr == WHITE) {
		_whitePieces |= squareToBitboard[sq];
	}
	else {
		_blackPieces |= squareToBitboard[sq];
	}
	_piecePos[p] |= squareToBitboard[sq];
}

// Removes a piece from all 4 occupation bitboards in the appropriate position
template <Color clr>
void PositionState::removePieceFromBitboards(Square sq, Piece p)
{
	if (clr == WHITE) {
		_whitePieces ^= squareToBitboard[sq];
	}
	else {
		_blackPieces ^= squareToBitboard[sq];
	}
	_piecePos[p] ^= squareToBitboard[sq];
}

int PositionState::calculatePstValue(Piece p, Square s) const
{
	if (_isMiddleGame) {
		return PST_MIDDLE_VALUE[p][s];
	}
	else {
		return PST_END_VALUE[p][s];
	}
}

// Updates the status of the game by setting 
// isMiddleGame variable true or false
// TODO: Needs improvement on classifing middle and end game
void PositionState::updateGameStatus()
{
	if (_pieceCount[QUEEN_WHITE] == 0 && _pieceCount[QUEEN_BLACK] == 0) {
		_isMiddleGame = false;
	}
}


void PositionState::undoMove()
{
	const UndoMoveInfo* move = _moveStack.pop();
	switch(move->moveType) {
		case NORMAL_MOVE: 
			undoNormalMove(*move);
			break;
		case CAPTURE_MOVE:
			undoCaptureMove(*move);
			break;
		case PROMOTION_MOVE:
			undoPromotionMove(*move);
			break;
		case CASTLING_MOVE:
			undoCastlingMove(*move);
			break;
		case EN_PASSANT_MOVE:
			undoEnPassantMove(*move);
			break;
		case EN_PASSANT_CAPTURE:
			undoEnPassantCapture(*move);
			break;
		default:
			assert(move->moveType >= NORMAL_MOVE && move->moveType <= EN_PASSANT_CAPTURE);
			break;
	}

	revertCastlingRights(*move);
	updateGameStatus();
	_isDoubleCheck = move->isDoubleCheck;
	_absolutePinsPos = move->absolutePinsPos;
	if (_isDoubleCheck) {
		_kingUnderCheck = true;
	}
	else {
	   if (_absolutePinsPos) {
		   _kingUnderCheck = true;
	   }
	   else {
		   _kingUnderCheck = false;
	   }
	}

	_occupiedSquares = _whitePieces | _blackPieces;
	_whiteToPlay = !_whiteToPlay;
	_zobKey ^= _zobKeyImpl->getIfBlackToPlayKey();
}

void PositionState::undoNormalMove(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {			
		removePieceFromBitboards<BLACK>(move.to, move.movedPiece);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
		if (move.movedPiece == KING_BLACK) {
			_blackKingPosition = move.from;
		}
	}
	else {
		removePieceFromBitboards<WHITE>(move.to, move.movedPiece);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
		if (move.movedPiece == KING_WHITE) {
			_whiteKingPosition = move.from;
		}
	}
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	_board[move.to / 8][move.to % 8] = ETY_SQUARE;
	_pstValue -= calculatePstValue(move.movedPiece, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
		_enPassantFile = move.enPassantFile;
	}
}

void PositionState::undoCaptureMove(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {			
		removePieceFromBitboards<BLACK>(move.to, move.movedPiece);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
		addPieceToBitboards<WHITE>(move.to, move.capturedPiece);
		if (move.movedPiece == KING_BLACK) {
			_blackKingPosition = move.from;
		}
	}
	else {
		removePieceFromBitboards<WHITE>(move.to, move.movedPiece);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
		addPieceToBitboards<BLACK>(move.to, move.capturedPiece);
		if (move.movedPiece == KING_WHITE) {
			_whiteKingPosition = move.from;
		}
	}
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	_board[move.to / 8][move.to % 8] = move.capturedPiece;
	_pstValue -= calculatePstValue(move.movedPiece, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);
	_pstValue += calculatePstValue(move.capturedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
	++_pieceCount[move.capturedPiece];
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
		_enPassantFile = move.enPassantFile;
	}
}

void PositionState::undoCastlingMove(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {
		assert(move.from == E8);
		assert(move.movedPiece == KING_BLACK);
		removePieceFromBitboards<BLACK>(move.to, move.movedPiece);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
		_board[move.from / 8][move.from % 8] = KING_BLACK;
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_pstValue -= calculatePstValue(KING_BLACK, move.to);
		_pstValue += calculatePstValue(KING_BLACK, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.from);
		_blackKingPosition = move.from;
		if (move.to == C8) {
			removePieceFromBitboards<BLACK>(D8, ROOK_BLACK);
			addPieceToBitboards<BLACK>(A8, ROOK_BLACK);
			_board[D8 / 8][D8 % 8] = ETY_SQUARE;
			_board[A8 / 8][A8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, D8);
			_pstValue += calculatePstValue(ROOK_BLACK, A8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, D8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, A8);
		}	
		else {
			assert(move.to == G8);
			removePieceFromBitboards<BLACK>(F8, ROOK_BLACK);
			addPieceToBitboards<BLACK>(H8, ROOK_BLACK);
			_board[F8 / 8][F8 % 8] = ETY_SQUARE;
			_board[H8 / 8][H8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, F8);
			_pstValue += calculatePstValue(ROOK_BLACK, H8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, F8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, H8);
		}
	}
	else {
		assert(move.from == E1);
		assert(move.movedPiece == KING_WHITE); 
		removePieceFromBitboards<WHITE>(move.to, move.movedPiece);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_board[move.from / 8][move.from % 8] = KING_WHITE;
		_pstValue -= calculatePstValue(KING_WHITE, move.to);
		_pstValue += calculatePstValue(KING_WHITE, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.from);
		_whiteKingPosition = move.from;
		if (move.to == C1) {
			removePieceFromBitboards<WHITE>(D1, ROOK_WHITE);
			addPieceToBitboards<WHITE>(A1, ROOK_WHITE);
			_board[D1 / 8][D1 % 8] = ETY_SQUARE;
			_board[A1 / 8][A1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, D1);
			_pstValue += calculatePstValue(ROOK_WHITE, A1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, D1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, A1);
		}	
		else {
			assert(move.to == G1);
			removePieceFromBitboards<WHITE>(F1, ROOK_WHITE);
			addPieceToBitboards<WHITE>(H1, ROOK_WHITE);
			_board[F1 / 8][F1 % 8] = ETY_SQUARE;
			_board[H1 / 8][H1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, F1);
			_pstValue += calculatePstValue(ROOK_WHITE, H1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, F1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, H1);

		}
	}
	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
		_enPassantFile = move.enPassantFile;	
	}	
}

void PositionState::undoEnPassantMove(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {
		removePieceFromBitboards<BLACK>(move.to, move.movedPiece);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
	}
	else {
		removePieceFromBitboards<WHITE>(move.to, move.movedPiece);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
	}
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = ETY_SQUARE;
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	_pstValue -= calculatePstValue(move.movedPiece, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getEnPassantKey(_enPassantFile);
	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
	}
	_enPassantFile = move.enPassantFile;
}

void PositionState::undoEnPassantCapture(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {
		removePieceFromBitboards<BLACK>(move.to, move.movedPiece);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
		addPieceToBitboards<WHITE>((Square) (move.to + 8), PAWN_WHITE);
		++_pieceCount[PAWN_WHITE];
		_board[move.to / 8 + 1][move.to % 8] = PAWN_WHITE;
		_pstValue += calculatePstValue(PAWN_WHITE, (Square) (move.to + 8));
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(PAWN_WHITE, (Square) (move.to + 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_WHITE, _pieceCount[PAWN_WHITE]);
	}
	else {
		removePieceFromBitboards<WHITE>(move.to, move.movedPiece);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
		addPieceToBitboards<BLACK>((Square) (move.to - 8), PAWN_BLACK);
		++_pieceCount[PAWN_BLACK];
		_board[move.to / 8 - 1][move.to % 8] = PAWN_BLACK;
		_pstValue += calculatePstValue(PAWN_BLACK, (Square) (move.to - 8));
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(PAWN_BLACK, (Square) (move.to - 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_BLACK, _pieceCount[PAWN_BLACK]);
	}
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = ETY_SQUARE;
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	_pstValue -= calculatePstValue(move.movedPiece, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
		_enPassantFile = move.enPassantFile;				
	}
}

void PositionState::undoPromotionMove(const UndoMoveInfo& move)
{
	Piece promoted = _board[move.to / 8][move.to % 8];
	if (_whiteToPlay) {
		assert(move.from >= A2 && move.from <= H2);
		removePieceFromBitboards<BLACK>(move.to, promoted);
		addPieceToBitboards<BLACK>(move.from, move.movedPiece);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards<WHITE>(move.to, move.capturedPiece);
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			++_pieceCount[move.capturedPiece];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
	}
	else {
		assert(move.from >= A7 && move.from <= H7);
		removePieceFromBitboards<WHITE>(move.to, promoted);
		addPieceToBitboards<WHITE>(move.from, move.movedPiece);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards<BLACK>(move.to, move.capturedPiece);
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			++_pieceCount[move.capturedPiece];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
	}
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = move.capturedPiece;
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	--_pieceCount[promoted];
	++_pieceCount[move.movedPiece];
	_pstValue -= calculatePstValue(promoted, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);

	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(promoted, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(promoted, _pieceCount[promoted] + 1);
	_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.movedPiece, _pieceCount[move.movedPiece]);

	if (move.enPassantFile != -1) {
		_zobKey ^= _zobKeyImpl->getEnPassantKey(move.enPassantFile);
		_enPassantFile = move.enPassantFile;
	}
}

void PositionState::revertCastlingRights(const UndoMoveInfo& move)
{

	if (!_whiteLeftCastling && move.whiteLeftCastling) {
		_zobKey ^= _zobKeyImpl->getWhiteLeftCastlingKey();
		_whiteLeftCastling = true;
	}
	if (!_whiteRightCastling && move.whiteRightCastling) {
		_zobKey ^= _zobKeyImpl->getWhiteRightCastlingKey();
		_whiteRightCastling = true;
	}
	if (!_blackLeftCastling && move.blackLeftCastling) {
		_zobKey ^= _zobKeyImpl->getBlackLeftCastlingKey();
		_blackLeftCastling = true;
	}
	if (!_blackRightCastling && move.blackRightCastling) {
		_zobKey ^= _zobKeyImpl->getBlackRightCastlingKey();
		_blackRightCastling = true;
	}
}
	 
PositionState::MoveStack::MoveStack() :
_stackSize(0)
{
}

bool PositionState::MoveStack::isEmpty() const
{
	return _stackSize == 0;
}

uint32_t PositionState::MoveStack::getSize() const
{
	return _stackSize;
}

// Returns the pointer to the next item which should be updated
// and then pushed into stack
// The value held in the variable to which the pointer points to is undefined  
PositionState::UndoMoveInfo* PositionState::MoveStack::getNextItem()
{
	assert(_stackSize != MOVE_STACK_CAPACITY);
	return _moveStack + (_stackSize++);
}

const PositionState::UndoMoveInfo* PositionState::MoveStack::pop()
{
	assert(!isEmpty());
	return _moveStack + (--_stackSize);
}

const std::string PositionState::getStateFEN() const
{
	std::string fen;
	constructMaterialFEN(fen);
	fen.push_back(' ');
	constructRightToPlayFEN(fen);
	fen.push_back(' ');
	constructCastlingRightsFEN(fen);
	fen.push_back(' ');
	constructEnPassantFileFEN(fen);
	fen.push_back(' ');
	constructMoveCountFEN(fen);

	return fen;
}

void PositionState::constructMaterialFEN(std::string& fen) const
{
	for (int rank = 7; rank >= 0; --rank) {
		unsigned int etySquareCount  = 0;
		for (unsigned int file = 0; file < 8; ++file) {
			Piece p = _board[rank][file];
			if(p != ETY_SQUARE) {
				if (etySquareCount != 0) {
					fen.push_back('0' + etySquareCount);
					etySquareCount = 0;
				}
				switch(p) {
					case PAWN_WHITE:
						fen.push_back('P');
						break;
					case KNIGHT_WHITE:
						fen.push_back('N');
						break;
					case BISHOP_WHITE:
						fen.push_back('B');
						break;
					case ROOK_WHITE:
						fen.push_back('R');
						break;
					case QUEEN_WHITE:
						fen.push_back('Q');
						break;
					case KING_WHITE:
						fen.push_back('K');
						break;
					case PAWN_BLACK:
						fen.push_back('p');
						break;
					case KNIGHT_BLACK:
						fen.push_back('n');
						break;
					case BISHOP_BLACK:
						fen.push_back('b');
						break;
					case ROOK_BLACK:
						fen.push_back('r');
						break;
					case QUEEN_BLACK:
						fen.push_back('q');
						break;
					case KING_BLACK:
						fen.push_back('k');
						break;
					default:
						assert(false);
				}
			}
			else {
				++etySquareCount;
			}
		}
		
		if (etySquareCount != 0) {
			fen.push_back('0' + etySquareCount);
			etySquareCount = 0;
		}
		if (rank != 0) {
			fen.push_back('/');
		}
	}
}

void PositionState::constructRightToPlayFEN(std::string& fen) const
{
	if (_whiteToPlay) {
		fen.push_back('w');
	}
	else {
		fen.push_back('b');
	}
}

void PositionState::constructCastlingRightsFEN(std::string& fen) const
{
	bool castlingAllowed = false;
	if (_whiteRightCastling) {
		fen.push_back('K');
		castlingAllowed = true;
	}
	if (_whiteLeftCastling) {
		fen.push_back('Q');
		castlingAllowed = true;
	}
	if (_blackRightCastling) {
		fen.push_back('k');
		castlingAllowed = true;
	}
	if (_blackLeftCastling) {
		fen.push_back('q');
		castlingAllowed = true;
	}
	if (!castlingAllowed) {
		fen.push_back('-');
	}
}

void PositionState::constructEnPassantFileFEN(std::string& fen) const
{
	if(_enPassantFile == -1) {
		fen.push_back('-');
	}
	else {
		fen.push_back('a' + _enPassantFile);
		if (_whiteToPlay) {
			fen.push_back('6');
		}
		else {
			fen.push_back('3');
		}
	}
}

void PositionState::constructMoveCountFEN(std::string& fen) const
{
	unsigned int halfmoveCount = _halfmoveClock;
	std::string hcRev;
	do {
		hcRev.push_back('0' + halfmoveCount % 10);
		halfmoveCount /= 10;
	} while (halfmoveCount != 0);
	for (int i = hcRev.size() - 1; i >= 0; --i) {
		fen.push_back(hcRev[i]);
	}
	fen.push_back(' ');
	unsigned int fullmoveCount = _fullmoveCount;
	std::string fcRev;
	do {
		fcRev.push_back('0' + fullmoveCount % 10);
		fullmoveCount /= 10;
	} while (fullmoveCount != 0);
	for (int i = fcRev.size() - 1; i >=0; --i) {
		fen.push_back(fcRev[i]);
	}
}


void PositionState::printWhitePieces() const
{
	std::cout << "White pieces:" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if ((_whitePieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_WHITE: std::cout << "P ";
						break;
					case KNIGHT_WHITE: std::cout << "N ";
						break;
					case BISHOP_WHITE: std::cout << "B ";
						break;
					case ROOK_WHITE: std::cout << "R ";
						break;
					case QUEEN_WHITE: std::cout << "Q ";
						break;
					case KING_WHITE: std::cout << "K ";
						break;
					default: std::cout << "X "; 
				}
			}
			else {
				std::cout << "E ";
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void PositionState::printBlackPieces() const
{
	std::cout << "Black pieces:" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if ((_blackPieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_BLACK: std::cout << "P ";
						break;
					case KNIGHT_BLACK: std::cout << "N ";
						break;
					case BISHOP_BLACK: std::cout << "B ";
						break;
					case ROOK_BLACK: std::cout << "R ";
						break;
					case QUEEN_BLACK: std::cout << "Q ";
						break;
					case KING_BLACK: std::cout << "K ";
						break;
					default: std::cout << "X ";
				}
			}
			else {
				std::cout << "E ";
			}

			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void PositionState::printBoard() const
{
	std::cout << "Complete board" << std::endl; 
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			switch(_board[i][j]) {
				case PAWN_WHITE: std::cout << "PW ";
					break;
				case KNIGHT_WHITE: std::cout << "NW ";
					break;
				case BISHOP_WHITE: std::cout << "BW ";
					break;
				case ROOK_WHITE: std::cout << "RW ";
					break;
				case QUEEN_WHITE: std::cout << "QW ";
					break;
				case KING_WHITE: std::cout << "KW ";
					break;
				case PAWN_BLACK: std::cout << "PB ";
					break;
				case KNIGHT_BLACK: std::cout << "NB ";
					break;
				case BISHOP_BLACK: std::cout << "BB ";
					break;
				case ROOK_BLACK: std::cout << "RB ";
					break;
				case QUEEN_BLACK: std::cout << "QB ";
					break;
				case KING_BLACK: std::cout << "KB ";
					break;
				case ETY_SQUARE: std::cout << "ES ";
					break;
				default: std::cout << "XX ";
					break;
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

}
