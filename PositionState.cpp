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
_whitePiecesTranspose(0),
_whitePiecesDiagA1h8(0),
_whitePiecesDiagA8h1(0),
_blackPieces(0),
_blackPiecesTranspose(0),
_blackPiecesDiagA1h8(0),
_blackPiecesDiagA8h1(0),
_bitboardImpl(new BitboardImpl()),
_zobKeyImpl(new ZobKeyImpl()),
_absolutePinsPos(0),
_attackedSquares(0),
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
		_pieceCount[i] = 0;
	} 
	
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
		_materialZobKey ^= _zobKeyImpl->getMaterialKey((Piece)(piece), 0);
	}
}

PositionState::~PositionState()
{
	delete _bitboardImpl;
	delete _zobKeyImpl;
}

void PositionState::setPiece(Square s, Piece p)
{
	_board[s / 8][s % 8] = p;
	if (p <= KING_WHITE) {
		if (p == KING_WHITE) {
			_whiteKingPosition = s;
		}
		addPieceToBitboards(s, WHITE);
	}
	else {
		if (p == KING_BLACK) {
			_blackKingPosition = s;
		}
		addPieceToBitboards(s, BLACK);
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

bool PositionState::moveIsPseudoLegal(const MoveInfo& move) const
{
	assert(move.from != move.to);
	Piece pfrom = _board[move.from  / 8][move.from % 8];
	if(pfrom == ETY_SQUARE) {
		return false;
	}
	bool isEnPassantCapture;
	if(_whiteToPlay) {
		if((_bitboardImpl->squareToBitboard(move.from) & _blackPieces) || (_bitboardImpl->squareToBitboard(move.to) & _whitePieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_WHITE: 
					return pawnMoveIsLegal(move, isEnPassantCapture);
				case KNIGHT_WHITE:
					return knightMoveIsLegal(move);
				case BISHOP_WHITE:
					return bishopMoveIsLegal(move);
				case ROOK_WHITE:
					return rookMoveIsLegal(move);
				case QUEEN_WHITE:
					return queenMoveIsLegal(move);
				case KING_WHITE:
					if (moveIsCastling(move)) {
						return castlingIsLegal(move);
					}
					else { 
						return kingMoveIsLegal(move);
					}
				default: 
					assert(pfrom >= PAWN_WHITE && pfrom <= KING_WHITE);
			}
		}
	}
	else {
		if((_bitboardImpl->squareToBitboard(move.from) & _whitePieces) || (_bitboardImpl->squareToBitboard(move.to) & _blackPieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_BLACK: 
					return pawnMoveIsLegal(move, isEnPassantCapture);
				case KNIGHT_BLACK: 
					return knightMoveIsLegal(move);
				case BISHOP_BLACK:
					return bishopMoveIsLegal(move);
				case ROOK_BLACK:
					return rookMoveIsLegal(move);
				case QUEEN_BLACK:
					return queenMoveIsLegal(move);
				case KING_BLACK:
					if (moveIsCastling(move)) { 
						return castlingIsLegal(move);
					}
					else { 
						return kingMoveIsLegal(move);
					}
				default: 
					assert(pfrom >= PAWN_BLACK && pfrom <= KING_BLACK);
			}
		}
	}
	return false;
}

bool PositionState::moveIsLegal(const MoveInfo& move) const
{
	assert(move.from != move.to);
	Piece pfrom = _board[move.from  / 8][move.from % 8];
	if(pfrom == ETY_SQUARE) {
		return false;
	}

	bool result = false;
	if(_whiteToPlay) {
		if((_bitboardImpl->squareToBitboard(move.from) & _blackPieces) || (_bitboardImpl->squareToBitboard(move.to) & _whitePieces)) {
			return false;
		}
		else {
			bool isEnPassantCapture = false;
			switch(pfrom) {
				case PAWN_WHITE: 
					result = pawnMoveIsLegal(move, isEnPassantCapture);
					break;
				case KNIGHT_WHITE:
					result = knightMoveIsLegal(move);
					break;
				case BISHOP_WHITE:
					result = bishopMoveIsLegal(move);
					break;
				case ROOK_WHITE:
					result = rookMoveIsLegal(move);
					break;
				case QUEEN_WHITE:
					result = queenMoveIsLegal(move);
					break;
				case KING_WHITE:
					if (moveIsCastling(move)) {
						return castlingIsLegal(move);
					}
					else { 
						result = kingMoveIsLegal(move);
					}
					break;
				default: 
					assert(pfrom >= PAWN_WHITE && pfrom <= KING_WHITE);
			}
	
			if (result) {
				Piece capturedPiece;
				makeLazyMove(move, isEnPassantCapture, capturedPiece);
				result = !(_bitboardImpl->squareToBitboard(_whiteKingPosition) & squaresUnderAttack(WHITE));
				undoLazyMove(move, isEnPassantCapture, capturedPiece);
			}
		}
	}
	else {
		if((_bitboardImpl->squareToBitboard(move.from) & _whitePieces) || (_bitboardImpl->squareToBitboard(move.to) & _blackPieces)) {
			return false;
		}
		else {
			bool isEnPassantCapture = false;
			switch(pfrom) {
				case PAWN_BLACK: 
					result = pawnMoveIsLegal(move, isEnPassantCapture);
					break;
				case KNIGHT_BLACK: 
					result = knightMoveIsLegal(move);
					break;
				case BISHOP_BLACK:
					result = bishopMoveIsLegal(move);
					break;
				case ROOK_BLACK:
					result = rookMoveIsLegal(move);
					break;
				case QUEEN_BLACK:
					result = queenMoveIsLegal(move);
					break;
				case KING_BLACK:
					if (moveIsCastling(move)) { 
						return castlingIsLegal(move);
					}
					else { 
						result = kingMoveIsLegal(move);
					}
					break;
				default: 
					return false;
			}
		
			if (result) {
				Piece capturedPiece;
				makeLazyMove(move, isEnPassantCapture, capturedPiece);
				result = !(_bitboardImpl->squareToBitboard(_blackKingPosition) & squaresUnderAttack(BLACK));
				undoLazyMove(move, isEnPassantCapture, capturedPiece);
			}
		}
	}

	return result;
}

bool PositionState::moveIsCastling(const MoveInfo& move) const
{
	if ((_whiteToPlay && _board[move.from / 8][move.from % 8] == KING_WHITE) ||
			(!_whiteToPlay && _board[move.from / 8][move.from % 8] == KING_BLACK)) {
		if (std::abs(move.from % 8 - move.to % 8) == 2) {
			return true;
		}
	}

	return false;
}

bool PositionState::moveIsEnPassantCapture(const MoveInfo& move) const
{
	if (_whiteToPlay) {
		if (_board[move.from / 8][move.from % 8] == PAWN_WHITE && _board[move.to / 8][move.to % 8] == ETY_SQUARE && (move.to - move.from) / 8 != 0) {
			return true;
		}
	}
	else {
		if (_board[move.from / 8][move.from % 8] == PAWN_BLACK && _board[move.to / 8][move.to % 8] == ETY_SQUARE && (move.from - move.to) / 8 != 0) {
			return true;
		}
	}
	return false;
}

bool PositionState::pieceIsSlidingPiece(Piece piece) const
{
	if (piece == ROOK_WHITE || piece == BISHOP_WHITE || piece == QUEEN_WHITE ||
			piece == ROOK_BLACK || piece == BISHOP_BLACK || piece == QUEEN_BLACK) {
		return true;
	}
	
	return false;
}

bool PositionState::pawnMoveIsLegal(const MoveInfo& move, bool& isEnPassantCapture) const
{
	isEnPassantCapture = false;
	if (_whiteToPlay) {
		assert(move.from > H1);
		// Checks for single square movement of the pawn
		if (move.to - move.from == 8 && !(_bitboardImpl->squareToBitboard(move.to) & _blackPieces)) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			return true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.to - move.from == 16 && (_bitboardImpl->squareToBitboard(move.from) & PAWN_WHITE_INIT) &&
		!(_bitboardImpl->squareToBitboard((Square)(move.from + 8)) & (_whitePieces | _blackPieces)) && !(_bitboardImpl->squareToBitboard(move.to) & _blackPieces)) {
			return true;
		}
		// Checks for usual capture movement of the pawn
		else if ((_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalPawnWhiteAttackingMoves(move.from)) && (_bitboardImpl->squareToBitboard(move.to) & _blackPieces)) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			return true;
		}
		else {
			isEnPassantCapture = true;
			return enPassantCaptureIsLegal(move);
		}
	}
	else {
		// Checks for single square movement of the pawn
		assert(move.from < A8);
		if (move.from - move.to == 8 && !(_bitboardImpl->squareToBitboard(move.to) & _whitePieces)) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			return true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.from - move.to == 16 && (_bitboardImpl->squareToBitboard(move.from) & PAWN_BLACK_INIT) &&
		!(_bitboardImpl->squareToBitboard((Square)(move.from - 8)) & (_whitePieces | _blackPieces)) && !(_bitboardImpl->squareToBitboard(move.to) & _whitePieces)) {
			return true;
		}
		// Checks for usual capture movement of the pawn
		else if ((_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalPawnBlackAttackingMoves(move.from)) && (_bitboardImpl->squareToBitboard(move.to) & _whitePieces)) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			return true;
		}
		else {
			isEnPassantCapture = true;
			return  enPassantCaptureIsLegal(move);
		}
	}
	return false;
}

bool PositionState::enPassantCaptureIsLegal(const MoveInfo& move) const
{
	if (_enPassantFile != -1) {
		if (_whiteToPlay) {
			assert(_board[4][_enPassantFile] == PAWN_BLACK);
			if (_enPassantFile == 0 && move.from == B5 && move.to == A6) {
				return true;
			}
			else if (_enPassantFile == 7 && move.from == G5 && move.to == H6) {
				return true;
			} 
			else if (((move.from == 4 * 8 + _enPassantFile + 1) || (move.from == 4 * 8 + _enPassantFile - 1))
			&& move.to == 5 * 8 + _enPassantFile) {
				return true;
			}
		}
		else {
			assert(_board[3][_enPassantFile] == PAWN_WHITE);
			if (_enPassantFile == 0 && move.from == B4 && move.to == A3) {
				return true;
			}
			else if (_enPassantFile == 7 && move.from == G4 && move.to == H3) {
				return true;
			}
			else if (((move.from == 3 * 8 + _enPassantFile + 1) || (move.from == 3 * 8 + _enPassantFile - 1))
			&& move.to == 2 * 8 + _enPassantFile) {
				return true;
			}
		}
 	}
	
	return false;
}

bool PositionState::knightMoveIsLegal(const MoveInfo& move) const
{
	if (_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalKnightMoves(move.from)) {
		return true;
	}
	return false;
}

bool PositionState::bishopMoveIsLegal(const MoveInfo& move) const
{
	if((_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _bitboardImpl->getLegalDiagA1h8Moves(move.from, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8))
	|| (_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _bitboardImpl->getLegalDiagA8h1Moves(move.from, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1))) {
		return true;
	}
	return false;
}

bool PositionState::rookMoveIsLegal(const MoveInfo& move) const
{
	if((_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalRankMoves(move.from, _whitePieces | _blackPieces))
	|| (_bitboardImpl->squareToBitboardTranspose(move.to) & _bitboardImpl->getLegalFileMoves(move.from, _whitePiecesTranspose | _blackPiecesTranspose))) {
		return true;
	}
	return false;
}

bool PositionState::queenMoveIsLegal(const MoveInfo& move) const
{
	if((_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalRankMoves(move.from, _whitePieces | _blackPieces))
	|| (_bitboardImpl->squareToBitboardTranspose(move.to) & _bitboardImpl->getLegalFileMoves(move.from, _whitePiecesTranspose | _blackPiecesTranspose)) 
	|| (_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _bitboardImpl->getLegalDiagA1h8Moves(move.from, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8))
	|| (_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _bitboardImpl->getLegalDiagA8h1Moves(move.from, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1))) {
		return true;
	}
	return false;
}

bool PositionState::kingMoveIsLegal(const MoveInfo& move) const
{
	if (_bitboardImpl->squareToBitboard(move.to) & _bitboardImpl->getLegalKingMoves(move.from)) {
		return true;
	}
	return false;
}

bool PositionState::castlingIsLegal(const MoveInfo& move) const
{
	if (_whiteToPlay) {
		if (move.from == E1 && move.to == C1) {
			if (_whiteLeftCastling && !(WHITE_LEFT_CASTLING_ETY_SQUARES & (_whitePieces | _blackPieces)) && !(WHITE_LEFT_CASTLING_KING_SQUARES & squaresUnderAttack(WHITE))) {
					return true;
				}
		}
		else if (move.from == E1 && move.to == G1) {
				if (_whiteRightCastling && !(WHITE_RIGHT_CASTLING_ETY_SQUARES & (_whitePieces | _blackPieces)) && !(WHITE_RIGHT_CASTLING_KING_SQUARES & squaresUnderAttack(WHITE))) {
					return true;
				}
		}
		
		return false;
	}
	else {
		if (move.from == E8 && move.to == C8) {
			if (_blackLeftCastling && !(BLACK_LEFT_CASTLING_ETY_SQUARES & (_whitePieces | _blackPieces)) && !(BLACK_LEFT_CASTLING_KING_SQUARES & squaresUnderAttack(BLACK))) {
					return true;
				}
		}
		else if (move.from == E8 && move.to == G8) {
				if (_blackRightCastling && !(BLACK_RIGHT_CASTLING_ETY_SQUARES & (_whitePieces | _blackPieces)) && !(BLACK_RIGHT_CASTLING_KING_SQUARES & squaresUnderAttack(BLACK))) {
					return true;
				}
		}
		
		return false;
	}
}

// Updates _attackedSquares bitboard to the bit set at
// the positions which are under attack for moving side
// In this calculation moving side king is removed from 
// occupied bitboard to eliminate self pinning
void PositionState::updateSquaresUnderAttack()
{
	Bitboard attackedBitboard = 0;
	Bitboard attackedBitboardTranspose = 0;
	Bitboard attackedBitboardDiagA1h8 = 0;
	Bitboard attackedBitboardDiagA8h1 = 0;
	
	Square kingSq = (_whiteToPlay) ? _whiteKingPosition : _blackKingPosition;
	Bitboard occupiedBitboard = (_whitePieces | _blackPieces) ^ _bitboardImpl->squareToBitboard(kingSq);
	Bitboard occupiedBitboardTranspose = (_whitePiecesTranspose | _blackPiecesTranspose) ^ _bitboardImpl->squareToBitboardTranspose(kingSq);
	Bitboard occupiedBitboardDiagA1h8 = (_whitePiecesDiagA1h8 | _blackPiecesDiagA1h8) ^ _bitboardImpl->squareToBitboardDiagA1h8(kingSq);
	Bitboard occupiedBitboardDiagA8h1 = (_whitePiecesDiagA8h1 | _blackPiecesDiagA8h1) ^ _bitboardImpl->squareToBitboardDiagA8h1(kingSq);
	
	if(_whiteToPlay) {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalPawnBlackAttackingMoves((Square) sq);
					break;
				case KNIGHT_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalKnightMoves((Square) sq);
					break;
				case BISHOP_BLACK:
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, occupiedBitboardDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, occupiedBitboardDiagA8h1);
					break;
				case ROOK_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, occupiedBitboard);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, occupiedBitboardTranspose);
					break;
				case QUEEN_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, occupiedBitboard);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, occupiedBitboardTranspose);
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, occupiedBitboardDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, occupiedBitboardDiagA8h1);
					break;
				case KING_BLACK:	
					attackedBitboard |= _bitboardImpl->getLegalKingMoves((Square) sq);
					break;
				default:
					break;
			}
		}
	}
	else {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalPawnWhiteAttackingMoves((Square) sq);
					break;
				case KNIGHT_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalKnightMoves((Square) sq);
					break;
				case BISHOP_WHITE:
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, occupiedBitboardDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, occupiedBitboardDiagA8h1);
					break;
				case ROOK_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, occupiedBitboard);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, occupiedBitboardTranspose);
					break;
				case QUEEN_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, occupiedBitboard);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, occupiedBitboardTranspose);
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, occupiedBitboardDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, occupiedBitboardDiagA8h1);
					break;
				case KING_WHITE:	
					attackedBitboard |= _bitboardImpl->getLegalKingMoves((Square) sq);
					break;
				default:
					break;
			}
		}
	}
		
	_attackedSquares = (attackedBitboard | _bitboardImpl->bitboardTransposeToBitboard(attackedBitboardTranspose) | _bitboardImpl->bitboardDiagA1h8ToBitboard(attackedBitboardDiagA1h8) | _bitboardImpl->bitboardDiagA8h1ToBitboard(attackedBitboardDiagA8h1));
}

// Returns a bitboard with the bit set at the positions where the 
// attackedColor pieces are under attack by opponent
Bitboard PositionState::squaresUnderAttack(Color attackedColor) const
{
	Bitboard attackedBitboard = 0;
	Bitboard attackedBitboardTranspose = 0;
	Bitboard attackedBitboardDiagA1h8 = 0;
	Bitboard attackedBitboardDiagA8h1 = 0;
	if(attackedColor == BLACK) {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalPawnWhiteAttackingMoves((Square) sq);
					break;
				case KNIGHT_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalKnightMoves((Square) sq);
					break;
				case BISHOP_WHITE:
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
					break;
				case ROOK_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, _whitePieces | _blackPieces);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, _whitePiecesTranspose | _blackPiecesTranspose);
					break;
				case QUEEN_WHITE:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, _whitePieces | _blackPieces);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, _whitePiecesTranspose | _blackPiecesTranspose);
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
					break;
				case KING_WHITE:	
					attackedBitboard |= _bitboardImpl->getLegalKingMoves((Square) sq);
					break;
				default:
					break;
			}
		}
	}
	else {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalPawnBlackAttackingMoves((Square) sq);
					break;
				case KNIGHT_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalKnightMoves((Square) sq);
					break;
				case BISHOP_BLACK:
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
					break;
				case ROOK_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, _whitePieces | _blackPieces);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, _whitePiecesTranspose | _blackPiecesTranspose);
					break;
				case QUEEN_BLACK:
					attackedBitboard |= _bitboardImpl->getLegalRankMoves((Square) sq, _whitePieces | _blackPieces);
					attackedBitboardTranspose |= _bitboardImpl->getLegalFileMoves((Square) sq, _whitePiecesTranspose | _blackPiecesTranspose);
					attackedBitboardDiagA1h8 |= _bitboardImpl->getLegalDiagA1h8Moves((Square) sq, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
					attackedBitboardDiagA8h1 |= _bitboardImpl->getLegalDiagA8h1Moves((Square) sq, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
					break;
				case KING_BLACK:	
					attackedBitboard |= _bitboardImpl->getLegalKingMoves((Square) sq);
					break;
				default:
					break;
			}
		}
	}	

	attackedBitboard |= (_bitboardImpl->bitboardTransposeToBitboard(attackedBitboardTranspose) | _bitboardImpl->bitboardDiagA1h8ToBitboard(attackedBitboardDiagA1h8) | _bitboardImpl->bitboardDiagA8h1ToBitboard(attackedBitboardDiagA8h1));

	if (attackedColor == BLACK) {
		return attackedBitboard & (~_whitePieces);
	}
	else {
		return attackedBitboard & (~_blackPieces);
	}
}

// Makes move by only updating the occupation bitboards and _board array
// Castling move and also promotion are not considered here,
// therefore pawns can appear at the first and last ranks due to this move
// This should be always used in conjunction with undoLazyMove 
void PositionState::makeLazyMove(const MoveInfo& move, bool isEnPassantCapture, Piece& capturedPiece) const
{
	PositionState * nonConstThis = const_cast<PositionState *>(this);
	capturedPiece = _board[move.to / 8][move.to % 8];
	if (_whiteToPlay) {
		if (isEnPassantCapture) {
			nonConstThis->removePieceFromBitboards((Square) (move.to - 8), BLACK);
			(nonConstThis->_board)[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
			}
		else {
			if(capturedPiece != ETY_SQUARE) {
				nonConstThis->removePieceFromBitboards(move.to, BLACK);
			}
		}
		nonConstThis->removePieceFromBitboards(move.from, WHITE);
		nonConstThis->addPieceToBitboards(move.to, WHITE);
	}
	else {
		if (isEnPassantCapture) {
			nonConstThis->removePieceFromBitboards((Square) (move.to + 8), WHITE);
			(nonConstThis->_board)[move.to / 8 + 1][move.to % 8] = ETY_SQUARE;
			}
		else {
			if(capturedPiece != ETY_SQUARE) {
				nonConstThis->removePieceFromBitboards(move.to, WHITE);
			}
		}
		nonConstThis->removePieceFromBitboards(move.from, BLACK);
		nonConstThis->addPieceToBitboards(move.to, BLACK);
	}
	Piece pfrom = _board[move.from / 8][move.from % 8]; 
	(nonConstThis->_board)[move.to / 8][move.to % 8] = pfrom;	
	(nonConstThis->_board)[move.from / 8][move.from % 8] = ETY_SQUARE;
	if (pfrom == KING_WHITE) {
		nonConstThis->_whiteKingPosition = move.to;
	}
	else if (pfrom == KING_BLACK) {
		nonConstThis->_blackKingPosition = move.to;
	}

}

// Reverses the lazy move done by makeLazyMove
void PositionState::undoLazyMove(const MoveInfo& move, bool isEnPassantCapture, Piece capturedPiece) const
{
	PositionState * nonConstThis = const_cast<PositionState *>(this);
	if (_whiteToPlay) {
		if (isEnPassantCapture) {
			nonConstThis->addPieceToBitboards((Square) (move.to - 8), BLACK);
			(nonConstThis->_board)[move.to / 8 - 1][move.to % 8] = PAWN_BLACK;
			}
		else {
			if(capturedPiece != ETY_SQUARE) {
				nonConstThis->addPieceToBitboards(move.to, BLACK);
			}
		}
		nonConstThis->addPieceToBitboards(move.from, WHITE);
		nonConstThis->removePieceFromBitboards(move.to, WHITE);
	}
	else {
		if (isEnPassantCapture) {
			nonConstThis->addPieceToBitboards((Square) (move.to + 8), WHITE);
			(nonConstThis->_board)[move.to / 8 + 1][move.to % 8] = PAWN_WHITE;
			}
		else {
			if(capturedPiece != ETY_SQUARE) {
				nonConstThis->addPieceToBitboards(move.to, WHITE);
			}
		}
		nonConstThis->addPieceToBitboards(move.from, BLACK);
		nonConstThis->removePieceFromBitboards(move.to, BLACK);
	}
	Piece pto = _board[move.to / 8][move.to % 8];
	(nonConstThis->_board)[move.from / 8][move.from % 8] = pto;
	(nonConstThis->_board)[move.to / 8][move.to % 8] = capturedPiece;
	if (pto == KING_WHITE) {
		nonConstThis->_whiteKingPosition = move.from;
	}
	else if (pto == KING_BLACK) {
		nonConstThis->_blackKingPosition = move.from;
	}
}

void PositionState::updateDirectCheckArray()
{
	if (_whiteToPlay) {
		_directCheck[KING_WHITE] = 0;
		_directCheck[KNIGHT_WHITE] = _bitboardImpl->getLegalKnightMoves(_blackKingPosition);
		_directCheck[PAWN_WHITE] = _bitboardImpl->getPawnWhiteCheckingPos(_blackKingPosition);
		Bitboard rankAttack = _bitboardImpl->getLegalRankMoves(_blackKingPosition, _whitePieces | _blackPieces);
		Bitboard fileAttack = _bitboardImpl->bitboardTransposeToBitboard(_bitboardImpl->getLegalFileMoves(_blackKingPosition, _whitePiecesTranspose | _blackPiecesTranspose));
		Bitboard diagA1h8Attack = _bitboardImpl->bitboardDiagA1h8ToBitboard(_bitboardImpl->getLegalDiagA1h8Moves(_blackKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8));
		Bitboard diagA8h1Attack = _bitboardImpl->bitboardDiagA8h1ToBitboard(_bitboardImpl->getLegalDiagA8h1Moves(_blackKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1));
		_directCheck[ROOK_WHITE] = rankAttack | fileAttack;
		_directCheck[BISHOP_WHITE] = diagA1h8Attack | diagA8h1Attack;
		_directCheck[QUEEN_WHITE] = rankAttack | fileAttack | diagA1h8Attack | diagA8h1Attack;
	}
	else {
		_directCheck[KING_BLACK] = 0;
		_directCheck[KNIGHT_BLACK] = _bitboardImpl->getLegalKnightMoves(_whiteKingPosition);
		_directCheck[PAWN_BLACK] = _bitboardImpl->getPawnBlackCheckingPos(_whiteKingPosition);
		Bitboard rankAttack = _bitboardImpl->getLegalRankMoves(_whiteKingPosition, _whitePieces | _blackPieces);
		Bitboard fileAttack = _bitboardImpl->bitboardTransposeToBitboard(_bitboardImpl->getLegalFileMoves(_whiteKingPosition, _whitePiecesTranspose | _blackPiecesTranspose));
		Bitboard diagA1h8Attack = _bitboardImpl->bitboardDiagA1h8ToBitboard(_bitboardImpl->getLegalDiagA1h8Moves(_whiteKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8));
		Bitboard diagA8h1Attack = _bitboardImpl->bitboardDiagA8h1ToBitboard(_bitboardImpl->getLegalDiagA8h1Moves(_whiteKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1));
		_directCheck[ROOK_BLACK] = rankAttack | fileAttack;
		_directCheck[BISHOP_BLACK] = diagA1h8Attack | diagA8h1Attack;
		_directCheck[QUEEN_BLACK] = rankAttack | fileAttack | diagA1h8Attack | diagA8h1Attack;
	}
}

void PositionState::updateDiscoveredChecksInfo()
{
	if (_whiteToPlay) {
		_stateDiscCheckInfo.rankPin = _bitboardImpl->getRankPinInfo(_blackKingPosition, _whitePieces | _blackPieces);
		_stateDiscCheckInfo.filePin = _bitboardImpl->getFilePinInfo(_blackKingPosition, _whitePiecesTranspose | _blackPiecesTranspose);
		_stateDiscCheckInfo.diagA1h8Pin = _bitboardImpl->getDiagA1h8PinInfo(_blackKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
		_stateDiscCheckInfo.diagA8h1Pin = _bitboardImpl->getDiagA8h1PinInfo(_blackKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
		updateNonDiagPinStatus(_stateDiscCheckInfo.rankPin, WHITE);
		updateNonDiagPinStatus(_stateDiscCheckInfo.filePin, WHITE);
		updateDiagPinStatus(_stateDiscCheckInfo.diagA1h8Pin, WHITE);
		updateDiagPinStatus(_stateDiscCheckInfo.diagA8h1Pin, WHITE);
	}
	else {
		_stateDiscCheckInfo.rankPin = _bitboardImpl->getRankPinInfo(_whiteKingPosition, _whitePieces | _blackPieces);
		_stateDiscCheckInfo.filePin = _bitboardImpl->getFilePinInfo(_whiteKingPosition, _whitePiecesTranspose | _blackPiecesTranspose);
		_stateDiscCheckInfo.diagA1h8Pin = _bitboardImpl->getDiagA1h8PinInfo(_whiteKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
		_stateDiscCheckInfo.diagA8h1Pin = _bitboardImpl->getDiagA8h1PinInfo(_whiteKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
		updateNonDiagPinStatus(_stateDiscCheckInfo.rankPin, BLACK);
		updateNonDiagPinStatus(_stateDiscCheckInfo.filePin, BLACK);
		updateDiagPinStatus(_stateDiscCheckInfo.diagA1h8Pin, BLACK);
		updateDiagPinStatus(_stateDiscCheckInfo.diagA8h1Pin, BLACK);
	}
}

void PositionState::updateNonDiagPinStatus(PinInfo& pin, Color clr) const
{
	Piece slidePiece;
	Piece queenPiece;
	if (clr == WHITE) {
		slidePiece = ROOK_WHITE;
		queenPiece = QUEEN_WHITE;
	}
	else {
		slidePiece = ROOK_BLACK;
		queenPiece = QUEEN_BLACK;
	}
	if (pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if (_board[pin.smallSlidingPiecePos / 8][pin.smallSlidingPiecePos % 8] != slidePiece && _board[pin.smallSlidingPiecePos / 8][pin.smallSlidingPiecePos % 8] != queenPiece) {
			pin.smallSlidingPiecePos = INVALID_SQUARE;
			pin.smallPinPos = 0;
		}
	}
	if (pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if (_board[pin.bigSlidingPiecePos / 8][pin.bigSlidingPiecePos % 8] != slidePiece && _board[pin.bigSlidingPiecePos / 8][pin.bigSlidingPiecePos % 8] != queenPiece) {
			pin.bigSlidingPiecePos = INVALID_SQUARE;
			pin.bigPinPos = 0;
		}
	}
}

void PositionState::updateDiagPinStatus(PinInfo& pin, Color clr) const
{
	Piece slidePiece;
	Piece queenPiece;
	if (clr == WHITE) {
		slidePiece = BISHOP_WHITE;
		queenPiece = QUEEN_WHITE;
	}
	else {
		slidePiece = BISHOP_BLACK;
		queenPiece = QUEEN_BLACK;
	}
	if (pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if (_board[pin.smallSlidingPiecePos / 8][pin.smallSlidingPiecePos % 8] != slidePiece && _board[pin.smallSlidingPiecePos / 8][pin.smallSlidingPiecePos % 8] != queenPiece) {
			pin.smallSlidingPiecePos = INVALID_SQUARE;
			pin.smallPinPos = 0;
		}
	}
	if (pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if (_board[pin.bigSlidingPiecePos / 8][pin.bigSlidingPiecePos % 8] != slidePiece && _board[pin.bigSlidingPiecePos / 8][pin.bigSlidingPiecePos % 8] != queenPiece) {
			pin.bigSlidingPiecePos = INVALID_SQUARE;
			pin.bigPinPos = 0;
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
		
	if (_bitboardImpl->squareToBitboard(move.to) & _directCheck[pfrom]) {
		if (pieceIsSlidingPiece(pfrom)) {
				_absolutePinsPos |= _bitboardImpl->getSquaresBetween(move.to, kingSq);
		}
		else {
			_absolutePinsPos |= _bitboardImpl->squareToBitboard(move.to);
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

	if (moveIsEnPassantCapture(move)) {
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
		if (_bitboardImpl->squareToBitboard(move.to) & _directCheck[move.promoted]) {
			if (_kingUnderCheck) {
				_isDoubleCheck = true;
				return;
			}
			_absolutePinsPos |= _bitboardImpl->getSquaresBetween(move.to, kingSq);
			_kingUnderCheck = true;
		}
	}
		
	if (moveIsCastling(move)) {
		if (castlingChecksOpponentKing(move, slidingPiecePos)) {
			_kingUnderCheck = true;
			_absolutePinsPos |= _bitboardImpl->getSquaresBetween(slidingPiecePos, kingSq);
		}
	}
}

bool PositionState::moveChecksOpponentKing(const MoveInfo& move) const
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_bitboardImpl->squareToBitboard(move.to) & _directCheck[pfrom]) {
		return true;
	}	   

	Square slidingPiecePos;	
	if (moveOpensDiscoveredCheck(move, slidingPiecePos)) {
		return true;
	}

	//TODO: there should be faster way than checking 8 conditions separately as below (e.g. comparing move.from with king pos)
	if (moveIsEnPassantCapture(move)) {
	   if (enPassantCaptureDiscoveresCheck(move, slidingPiecePos)) {
		   return true;
	   }
	}	   

	if (move.promoted != ETY_SQUARE) {
		if (_bitboardImpl->squareToBitboard(move.to) & _directCheck[move.promoted]) {
			return true;
		}
	}
		
	if (moveIsCastling(move)) {
		if (castlingChecksOpponentKing(move, slidingPiecePos)) {
			return true;
		}
	}

	return false;
}

bool PositionState::moveOpensDiscoveredCheck(const MoveInfo& move, Square& slidingPiecePos) const
{
	slidingPiecePos = INVALID_SQUARE;
	if (_whiteToPlay) {
		if (!(_bitboardImpl->squareToBitboard(move.from) & _bitboardImpl->getSlidingPieceMoves(_blackKingPosition))) {
			return false;
		}
	}
	else {
		if (!(_bitboardImpl->squareToBitboard(move.from) & _bitboardImpl->getSlidingPieceMoves(_whiteKingPosition))) {
			return false;
		}
	}

	if (_stateDiscCheckInfo.rankPin.smallSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.rankPin.smallSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboard(move.from) & _stateDiscCheckInfo.rankPin.smallPinPos) && !(_bitboardImpl->squareToBitboard(move.to) & _stateDiscCheckInfo.rankPin.smallPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.rankPin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.rankPin.bigSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.rankPin.bigSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboard(move.from) & _stateDiscCheckInfo.rankPin.bigPinPos) && !(_bitboardImpl->squareToBitboard(move.to) & _stateDiscCheckInfo.rankPin.bigPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.rankPin.bigSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.filePin.smallSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.filePin.smallSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardTranspose(move.from) & _stateDiscCheckInfo.filePin.smallPinPos) && !(_bitboardImpl->squareToBitboardTranspose(move.to) & _stateDiscCheckInfo.filePin.smallPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.filePin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.filePin.bigSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.filePin.bigSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardTranspose(move.from) & _stateDiscCheckInfo.filePin.bigPinPos) && !(_bitboardImpl->squareToBitboardTranspose(move.to) & _stateDiscCheckInfo.filePin.bigPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.filePin.bigSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA1h8Pin.smallSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.diagA1h8Pin.smallSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardDiagA1h8(move.from) & _stateDiscCheckInfo.diagA1h8Pin.smallPinPos) && !(_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _stateDiscCheckInfo.diagA1h8Pin.smallPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.diagA1h8Pin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA1h8Pin.bigSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.diagA1h8Pin.bigSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardDiagA1h8(move.from) & _stateDiscCheckInfo.diagA1h8Pin.bigPinPos) && !(_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _stateDiscCheckInfo.diagA1h8Pin.bigPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.diagA1h8Pin.bigSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA8h1Pin.smallSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.diagA8h1Pin.smallSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardDiagA8h1(move.from) & _stateDiscCheckInfo.diagA8h1Pin.smallPinPos) && !(_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _stateDiscCheckInfo.diagA8h1Pin.smallPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.diagA8h1Pin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA8h1Pin.bigSlidingPiecePos != INVALID_SQUARE && move.from != _stateDiscCheckInfo.diagA8h1Pin.bigSlidingPiecePos) {
		if ((_bitboardImpl->squareToBitboardDiagA8h1(move.from) & _stateDiscCheckInfo.diagA8h1Pin.bigPinPos) && !(_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _stateDiscCheckInfo.diagA8h1Pin.bigPinPos)) {
			slidingPiecePos = _stateDiscCheckInfo.diagA8h1Pin.bigSlidingPiecePos;
			return true;
		}
	}

	return false;
}

bool PositionState::castlingChecksOpponentKing(const MoveInfo& move, Square& slidingPiecePos) const
{
	if (_whiteToPlay) {
		if (_blackKingPosition / 8 != 0) {
			if (move.to == C1) {
				if (_bitboardImpl->squareToBitboard(D1) & _directCheck[ROOK_WHITE]) {
					slidingPiecePos = D1;
					return true;
				}
			}
			else {
				assert (move.to == G1);
				if (_bitboardImpl->squareToBitboard(F1) & _directCheck[ROOK_WHITE]) {
					slidingPiecePos = F1;
					return true;
				}
			}
		}
		else {
			if (_bitboardImpl->squareToBitboard(E1) & _directCheck[ROOK_WHITE]) {
				slidingPiecePos = (move.to > move.from) ? F1 : D1;
				return true;
			}
		}
	}
	else {
		if (_whiteKingPosition / 8 != 7) {
			if (move.to == C8) {
				if (_bitboardImpl->squareToBitboard(D8) & _directCheck[ROOK_BLACK]) {
					slidingPiecePos = D8;
					return true;
				}
			}
			else {
				assert (move.to == G8);
				if (_bitboardImpl->squareToBitboard(F8) & _directCheck[ROOK_BLACK]) {
					slidingPiecePos = F8;
					return true;
				}
			}
		}
		else {
			if (_bitboardImpl->squareToBitboard(E8) & _directCheck[ROOK_WHITE]) {
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
	if (_whiteToPlay && _blackKingPosition / 8 == 4) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _whitePieces | _blackPieces, leftPos, rightPos);
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
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _whitePieces | _blackPieces, leftPos, rightPos);
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
	
	slidingPiecePos = INVALID_SQUARE;
	if (_whiteToPlay) {
		if (!(_bitboardImpl->squareToBitboard(capturedPawnPos) & _bitboardImpl->getSlidingPieceMoves(_blackKingPosition))) {
			return false;
		}
	}
	else {
		if (!(_bitboardImpl->squareToBitboard(capturedPawnPos) & _bitboardImpl->getSlidingPieceMoves(_whiteKingPosition))) {
			return false;
		}
	}
	
	if (_stateDiscCheckInfo.diagA1h8Pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if (_bitboardImpl->squareToBitboardDiagA1h8(capturedPawnPos) & _stateDiscCheckInfo.diagA1h8Pin.smallPinPos) {
			slidingPiecePos = _stateDiscCheckInfo.diagA1h8Pin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA1h8Pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if (_bitboardImpl->squareToBitboardDiagA1h8(capturedPawnPos) & _stateDiscCheckInfo.diagA1h8Pin.bigPinPos) {
			slidingPiecePos = _stateDiscCheckInfo.diagA1h8Pin.bigSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA8h1Pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if (_bitboardImpl->squareToBitboardDiagA8h1(capturedPawnPos) & _stateDiscCheckInfo.diagA8h1Pin.smallPinPos) {
			slidingPiecePos = _stateDiscCheckInfo.diagA8h1Pin.smallSlidingPiecePos;
			return true;
		}
	}
	if (_stateDiscCheckInfo.diagA8h1Pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if (_bitboardImpl->squareToBitboardDiagA8h1(capturedPawnPos) & _stateDiscCheckInfo.diagA8h1Pin.bigPinPos) {
			slidingPiecePos = _stateDiscCheckInfo.diagA8h1Pin.bigSlidingPiecePos;
			return true;
		}
	}
	return false;
}

void PositionState::updateStatePinInfo()
{
	if (_whiteToPlay) {
		_statePinInfo.rankPin = _bitboardImpl->getRankPinInfo(_whiteKingPosition, _whitePieces | _blackPieces);
		_statePinInfo.filePin = _bitboardImpl->getFilePinInfo(_whiteKingPosition, _whitePiecesTranspose | _blackPiecesTranspose);
		_statePinInfo.diagA1h8Pin = _bitboardImpl->getDiagA1h8PinInfo(_whiteKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
		_statePinInfo.diagA8h1Pin = _bitboardImpl->getDiagA8h1PinInfo(_whiteKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
		updateNonDiagPinStatus(_statePinInfo.rankPin, BLACK);
		updateNonDiagPinStatus(_statePinInfo.filePin, BLACK);
		updateDiagPinStatus(_statePinInfo.diagA1h8Pin, BLACK);
		updateDiagPinStatus(_statePinInfo.diagA8h1Pin, BLACK);
	}
	else {
		_statePinInfo.rankPin = _bitboardImpl->getRankPinInfo(_blackKingPosition, _whitePieces | _blackPieces);
		_statePinInfo.filePin = _bitboardImpl->getFilePinInfo(_blackKingPosition, _whitePiecesTranspose | _blackPiecesTranspose);
		_statePinInfo.diagA1h8Pin = _bitboardImpl->getDiagA1h8PinInfo(_blackKingPosition, _whitePiecesDiagA1h8 | _blackPiecesDiagA1h8);
		_statePinInfo.diagA8h1Pin = _bitboardImpl->getDiagA8h1PinInfo(_blackKingPosition, _whitePiecesDiagA8h1 | _blackPiecesDiagA8h1);
		updateNonDiagPinStatus(_statePinInfo.rankPin, WHITE);
		updateNonDiagPinStatus(_statePinInfo.filePin, WHITE);
		updateDiagPinStatus(_statePinInfo.diagA1h8Pin, WHITE);
		updateDiagPinStatus(_statePinInfo.diagA8h1Pin, WHITE);
	}
}

bool PositionState::isEvasionMove(const MoveInfo& move) const
{
	if (_whiteToPlay && _board[move.from / 8][move.from % 8] == KING_WHITE) {
		return true;
	}
	else if (!_whiteToPlay && _board[move.from / 8][move.from % 8] == KING_BLACK) {
		return true;
	}
	else if (!_isDoubleCheck) {
		if (_bitboardImpl->squareToBitboard(move.to) && _absolutePinsPos) {
			return true;
		}
	}

	return false;
}

bool PositionState::pseudomoveIsLegalMove(const MoveInfo& move) const
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	
	if (pfrom == KING_WHITE || pfrom == KING_BLACK) {
		if (_bitboardImpl->squareToBitboard(move.to) & _attackedSquares) {
			return false;
		}
		else {
			return true;
		}
	}

	if (_kingUnderCheck && !isEvasionMove(move)) {
		return false;
	}

	if (!pinMoveOpensCheck(move) || !pinEnPassantMoveOpensCheck(move)) {
		return true;
	}

	return false;
}

bool PositionState::pinMoveOpensCheck(const MoveInfo& move) const
{
	if (_whiteToPlay) {
		if (!(_bitboardImpl->squareToBitboard(move.from) & _bitboardImpl->getSlidingPieceMoves(_whiteKingPosition))) {
			return false;
		}
	}
	else {
		if (!(_bitboardImpl->squareToBitboard(move.from) & _bitboardImpl->getSlidingPieceMoves(_blackKingPosition))) {
			return false;
		}
	}

	//TODO: there should be faster way than checking 8 conditions separately as below (e.g. comparing move.from with king pos)
	if (_statePinInfo.rankPin.smallSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboard(move.from) & _statePinInfo.rankPin.smallPinPos) && !(_bitboardImpl->squareToBitboard(move.to) & _statePinInfo.rankPin.smallPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.rankPin.bigSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboard(move.from) & _statePinInfo.rankPin.bigPinPos) && !(_bitboardImpl->squareToBitboard(move.to) & _statePinInfo.rankPin.bigPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.filePin.smallSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardTranspose(move.from) & _statePinInfo.filePin.smallPinPos) && !(_bitboardImpl->squareToBitboardTranspose(move.to) & _statePinInfo.filePin.smallPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.filePin.bigSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardTranspose(move.from) & _statePinInfo.filePin.bigPinPos) && !(_bitboardImpl->squareToBitboardTranspose(move.to) & _statePinInfo.filePin.bigPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.diagA1h8Pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardDiagA1h8(move.from) & _statePinInfo.diagA1h8Pin.smallPinPos) && !(_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _statePinInfo.diagA1h8Pin.smallPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.diagA1h8Pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardDiagA1h8(move.from) & _statePinInfo.diagA1h8Pin.bigPinPos) && !(_bitboardImpl->squareToBitboardDiagA1h8(move.to) & _statePinInfo.diagA1h8Pin.bigPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.diagA8h1Pin.smallSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardDiagA8h1(move.from) & _statePinInfo.diagA8h1Pin.smallPinPos) && !(_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _statePinInfo.diagA8h1Pin.smallPinPos)) {
			return true;
		}
	}
	if (_statePinInfo.diagA8h1Pin.bigSlidingPiecePos != INVALID_SQUARE) {
		if ((_bitboardImpl->squareToBitboardDiagA8h1(move.from) & _statePinInfo.diagA8h1Pin.bigPinPos) && !(_bitboardImpl->squareToBitboardDiagA8h1(move.to) & _statePinInfo.diagA8h1Pin.bigPinPos)) {
			return true;
		}
	}

	return false;
}

bool PositionState::pinEnPassantMoveOpensCheck(const MoveInfo& move) const
{
	if (_whiteToPlay && _whiteKingPosition / 8 == 4) {
		Square leftPos;
		Square rightPos;
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _whitePieces | _blackPieces, leftPos, rightPos);
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
		_bitboardImpl->getEnPassantPinInfo(move.from, move.to, _whitePieces | _blackPieces, leftPos, rightPos);
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
	
	updateMoveChecksOpponentKing(move);	

	if (pfrom == PAWN_WHITE || pfrom == PAWN_BLACK) {
		if(move.to >= A8 || move.to <= H1) {
			makePromotionMove(move);
			undoMove->moveType = PROMOTION_MOVE;
		}
		else if (std::abs(move.from - move.to) == 16) {
			makeEnPassantMove(move);
			undoMove->moveType = EN_PASSANT_MOVE;
		}
		else if (moveIsEnPassantCapture(move)) {
			makeEnPassantCapture(move);
			undoMove->moveType = EN_PASSANT_CAPTURE;
		}
		else {
			makeNormalMove(move);
			undoMove->moveType = NORMAL_MOVE;
		}
	}
	else if (moveIsCastling(move)) {
		makeCastlingMove(move);
		undoMove->moveType = CASTLING_MOVE;
	}		
	else {
		makeNormalMove(move);
		undoMove->moveType = NORMAL_MOVE;
	}

	updateCastlingRights(move);
	updateGameStatus();
	
	_whiteToPlay = !_whiteToPlay;
	_zobKey ^= _zobKeyImpl->getIfBlackToPlayKey();
}

void PositionState::makeNormalMove(const MoveInfo& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (_whiteToPlay) {			
		removePieceFromBitboards(move.from, WHITE);
		addPieceToBitboards(move.to, WHITE);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards(move.to, BLACK);
			_pstValue -= calculatePstValue(pto, move.to);
			--_pieceCount[pto];
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
		}
		if (pfrom == KING_WHITE) {
			_whiteKingPosition = move.to;
		}
	}
	else {
		removePieceFromBitboards(move.from, BLACK);
		addPieceToBitboards(move.to, BLACK);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards(move.to, WHITE);
			_pstValue -= calculatePstValue(pto, move.to);
			--_pieceCount[pto];
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
		}
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

//Castling is assumed to be King's move
void PositionState::makeCastlingMove(const MoveInfo& move)
{
	if (_whiteToPlay) {
		assert(move.from == E1);
		removePieceFromBitboards(move.from, WHITE);
		addPieceToBitboards(move.to, WHITE);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_WHITE;
		_pstValue -= calculatePstValue(KING_WHITE, move.from);
		_pstValue += calculatePstValue(KING_WHITE, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.to);
		_whiteKingPosition = move.to;
		if (move.to == C1) {
			removePieceFromBitboards(A1, WHITE);
			addPieceToBitboards(D1, WHITE);	
			_board[A1 / 8][A1 % 8] = ETY_SQUARE;
			_board[D1 / 8][D1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, A1);
			_pstValue += calculatePstValue(ROOK_WHITE, D1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, A1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, D1);
		}	
		else {
			assert(move.to == G1);
			removePieceFromBitboards(H1, WHITE);
			addPieceToBitboards(F1, WHITE);	
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
		removePieceFromBitboards(move.from, BLACK);
		addPieceToBitboards(move.to, BLACK);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_BLACK;
		_pstValue -= calculatePstValue(KING_BLACK, move.from);
		_pstValue += calculatePstValue(KING_BLACK, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.to);
		_blackKingPosition = move.to;
		if (move.to == C8) {
			removePieceFromBitboards(A8, BLACK);
			addPieceToBitboards(D8, BLACK);	
			_board[A8 / 8][A8 % 8] = ETY_SQUARE;
			_board[D8 / 8][D8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, A8);
			_pstValue += calculatePstValue(ROOK_BLACK, D8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, A8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, D8);
		}	
		else {
			assert(move.to == G8);
			removePieceFromBitboards(H8, BLACK);
			addPieceToBitboards(F8, BLACK);	
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
		removePieceFromBitboards(move.from, WHITE);
		addPieceToBitboards(move.to, WHITE);
	}
	else {
		removePieceFromBitboards(move.from, BLACK);
		addPieceToBitboards(move.to, BLACK);
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
		removePieceFromBitboards(move.from, WHITE);
		addPieceToBitboards(move.to, WHITE);
		removePieceFromBitboards((Square) (move.to - 8), BLACK);
		--_pieceCount[PAWN_BLACK];
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
		_pstValue -= calculatePstValue(PAWN_BLACK, (Square) (move.to - 8));
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(PAWN_BLACK, (Square) (move.to - 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_BLACK, _pieceCount[PAWN_BLACK] + 1);
	}
	else {
		removePieceFromBitboards(move.from, BLACK);
		addPieceToBitboards(move.to, BLACK);
		removePieceFromBitboards((Square) (move.to + 8), WHITE);
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
		removePieceFromBitboards(move.from, WHITE);
		addPieceToBitboards(move.to, WHITE);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards(move.to, BLACK);
			_pstValue -= calculatePstValue(pto, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(pto, move.to);
			--_pieceCount[pto];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(pto, _pieceCount[pto] + 1);
		}
	}
	else {
		assert(move.from >= A2 && move.from <= H2);
		removePieceFromBitboards(move.from, BLACK);
		addPieceToBitboards(move.to, BLACK);
		if (pto != ETY_SQUARE) {
			removePieceFromBitboards(move.to, WHITE);
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
void PositionState::addPieceToBitboards(Square sq, Color clr)
{
	assert(clr == WHITE || clr == BLACK);
	if (clr == WHITE) {
		_whitePieces |= _bitboardImpl->squareToBitboard(sq);
		_whitePiecesTranspose |= _bitboardImpl->squareToBitboardTranspose(sq);
		_whitePiecesDiagA1h8 |= _bitboardImpl->squareToBitboardDiagA1h8(sq);
		_whitePiecesDiagA8h1 |= _bitboardImpl->squareToBitboardDiagA8h1(sq);
	}
	else {
		_blackPieces |= _bitboardImpl->squareToBitboard(sq);
		_blackPiecesTranspose |= _bitboardImpl->squareToBitboardTranspose(sq);
		_blackPiecesDiagA1h8 |= _bitboardImpl->squareToBitboardDiagA1h8(sq);
		_blackPiecesDiagA8h1 |= _bitboardImpl->squareToBitboardDiagA8h1(sq);
	}
	
}

// Removes a piece from all 4 occupation bitboards in the appropriate position
void PositionState::removePieceFromBitboards(Square sq, Color clr)
{
	assert(clr == WHITE || clr == BLACK);
	if (clr == WHITE) {
		_whitePieces ^= _bitboardImpl->squareToBitboard(sq);
		_whitePiecesTranspose ^= _bitboardImpl->squareToBitboardTranspose(sq);
		_whitePiecesDiagA1h8 ^= _bitboardImpl->squareToBitboardDiagA1h8(sq);
		_whitePiecesDiagA8h1 ^= _bitboardImpl->squareToBitboardDiagA8h1(sq);
	}
	else {
		_blackPieces ^= _bitboardImpl->squareToBitboard(sq);
		_blackPiecesTranspose ^= _bitboardImpl->squareToBitboardTranspose(sq);
		_blackPiecesDiagA1h8 ^= _bitboardImpl->squareToBitboardDiagA1h8(sq);
		_blackPiecesDiagA8h1 ^= _bitboardImpl->squareToBitboardDiagA8h1(sq);
	}
	
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
	
	_whiteToPlay = !_whiteToPlay;
	_zobKey ^= _zobKeyImpl->getIfBlackToPlayKey();
}

void PositionState::undoNormalMove(const UndoMoveInfo& move)
{
	if (_whiteToPlay) {			
		removePieceFromBitboards(move.to, BLACK);
		addPieceToBitboards(move.from, BLACK);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards(move.to, WHITE);
			++_pieceCount[move.capturedPiece];
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
		if (move.movedPiece == KING_BLACK) {
			_blackKingPosition = move.from;
		}
	}
	else {
		removePieceFromBitboards(move.to, WHITE);
		addPieceToBitboards(move.from, WHITE);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards(move.to, BLACK);
			++_pieceCount[move.capturedPiece];
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
		if (move.movedPiece == KING_WHITE) {
			_whiteKingPosition = move.from;
		}
	}
	_board[move.from / 8][move.from % 8] = move.movedPiece;
	_board[move.to / 8][move.to % 8] = move.capturedPiece;
	_pstValue -= calculatePstValue(move.movedPiece, move.to);
	_pstValue += calculatePstValue(move.movedPiece, move.from);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.to);
	_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.movedPiece, move.from);
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
		removePieceFromBitboards(move.to, BLACK);
		addPieceToBitboards(move.from, BLACK);
		_board[move.from / 8][move.from % 8] = KING_BLACK;
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_pstValue -= calculatePstValue(KING_BLACK, move.to);
		_pstValue += calculatePstValue(KING_BLACK, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_BLACK, move.from);
		_blackKingPosition = move.from;
		if (move.to == C8) {
			removePieceFromBitboards(D8, BLACK);
			addPieceToBitboards(A8, BLACK);	
			_board[D8 / 8][D8 % 8] = ETY_SQUARE;
			_board[A8 / 8][A8 % 8] = ROOK_BLACK;
			_pstValue -= calculatePstValue(ROOK_BLACK, D8);
			_pstValue += calculatePstValue(ROOK_BLACK, A8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, D8);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_BLACK, A8);
		}	
		else {
			assert(move.to == G8);
			removePieceFromBitboards(F8, BLACK);
			addPieceToBitboards(H8, BLACK);	
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
		removePieceFromBitboards(move.to, WHITE);
		addPieceToBitboards(move.from, WHITE);
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_board[move.from / 8][move.from % 8] = KING_WHITE;
		_pstValue -= calculatePstValue(KING_WHITE, move.to);
		_pstValue += calculatePstValue(KING_WHITE, move.from);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.to);
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(KING_WHITE, move.from);
		_whiteKingPosition = move.from;
		if (move.to == C1) {
			removePieceFromBitboards(D1, WHITE);
			addPieceToBitboards(A1, WHITE);	
			_board[D1 / 8][D1 % 8] = ETY_SQUARE;
			_board[A1 / 8][A1 % 8] = ROOK_WHITE;
			_pstValue -= calculatePstValue(ROOK_WHITE, D1);
			_pstValue += calculatePstValue(ROOK_WHITE, A1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, D1);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(ROOK_WHITE, A1);
		}	
		else {
			assert(move.to == G1);
			removePieceFromBitboards(F1, WHITE);
			addPieceToBitboards(H1, WHITE);	
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
		removePieceFromBitboards(move.to, BLACK);
		addPieceToBitboards(move.from, BLACK);
	}
	else {
		removePieceFromBitboards(move.to, WHITE);
		addPieceToBitboards(move.from, WHITE);
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
		removePieceFromBitboards(move.to, BLACK);
		addPieceToBitboards(move.from, BLACK);
		addPieceToBitboards((Square) (move.to + 8), WHITE);
		++_pieceCount[PAWN_WHITE];
		_board[move.to / 8 + 1][move.to % 8] = PAWN_WHITE;
		_pstValue += calculatePstValue(PAWN_WHITE, (Square) (move.to + 8));
		_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(PAWN_WHITE, (Square) (move.to + 8));
		_materialZobKey ^= _zobKeyImpl->getMaterialKey(PAWN_WHITE, _pieceCount[PAWN_WHITE]);
	}
	else {
		removePieceFromBitboards(move.to, WHITE);
		addPieceToBitboards(move.from, WHITE);
		addPieceToBitboards((Square) (move.to - 8), BLACK);
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
	if (_whiteToPlay) {
		assert(move.from >= A2 && move.from <= H2);
		removePieceFromBitboards(move.to, BLACK);
		addPieceToBitboards(move.from, BLACK);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards(move.to, WHITE);
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			++_pieceCount[move.capturedPiece];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
	}
	else {
		assert(move.from >= A7 && move.from <= H7);
		removePieceFromBitboards(move.to, WHITE);
		addPieceToBitboards(move.from, WHITE);
		if (move.capturedPiece != ETY_SQUARE) {
			addPieceToBitboards(move.to, BLACK);
			_pstValue += calculatePstValue(move.capturedPiece, move.to);
			_zobKey ^= _zobKeyImpl->getPieceAtSquareKey(move.capturedPiece, move.to);
			++_pieceCount[move.capturedPiece];
			_materialZobKey ^= _zobKeyImpl->getMaterialKey(move.capturedPiece, _pieceCount[move.capturedPiece]);
		}
	}
	Piece promoted = _board[move.to / 8][move.to % 8];
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

void PositionState::printPossibleMoves(Square from) const
{
	Piece promoted;
	if (_whiteToPlay) {
		promoted = QUEEN_WHITE;
	}
	else {
		promoted = QUEEN_BLACK;
	}
	std::cout << "Possible moves" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			MoveInfo move = {from, (Square) (i * 8 + j), promoted};
			if (moveIsLegal(move)) {
				std::cout << "L ";
			}
			else {
				std::cout << "X ";
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}
}
