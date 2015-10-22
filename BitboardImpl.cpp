#include "BitboardImpl.h"
#include "PossibleMoves.h"
#include <vector>
#include <assert.h>

namespace pismo
{

BitboardImpl::BitboardImpl()
{
	initSquareToBitboard();
	initSquareToBitboardTranspose();
	initSquareToBitboardA1h8();
	initSquareToBitboardA8h1();

	initMovePosBoardRank();
	initMovePosBoardFile();
	initMovePosBoardA1h8();
	initMovePosBoardA8h1();

	initMovePosBoardKnight();
	initMovePosBoardKing();
	initAttackingPosBoardPawnWhite();
	initAttackingPosBoardPawnBlack();

	initRankPinInfo();
	initFilePinInfo();
	initDiagA1h8PinInfo();
	initDiagA8h1PinInfo();

	initSlidingPosBoard();

	initSquaresBetween();
}

// Returns the bitboard with sq Square filled 
Bitboard BitboardImpl::squareToBitboard(Square sq) const
{
	return _squareToBitboard[sq];
}

// Returns the transposed bitboard with sq Square filled
Bitboard BitboardImpl::squareToBitboardTranspose(Square sq) const
{
	return _squareToBitboardTranspose[sq];
}

// Returns the 45 angle rotated bitboard with sq Square filled
Bitboard BitboardImpl::squareToBitboardDiagA1h8(Square sq) const
{
	return _squareToBitboardA1h8[sq];
}

// Returns the -45 angle rotated bitboard with sq Square filled
Bitboard BitboardImpl::squareToBitboardDiagA8h1(Square sq) const
{
	return _squareToBitboardA8h1[sq];
}

// Returns the bitboard for possible rank moves from Square from. 
// The occupiedSquares bitboard is the unrotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::getLegalRankMoves(Square from, const Bitboard& occupiedSquares) const
{
	Bitrank rankOccup = occupiedSquares >> (from / 8) * 8;
	return _movePosBoardRank[from][rankOccup];
}  

// Returns the transposed bitboard for possible file moves from Square from. 
// The occupiedSquares bitboard is the transposed bitboard with occupied squares bit set to one
Bitboard BitboardImpl::getLegalFileMoves(Square from, const Bitboard& occupiedSquares) const
{
	Bitrank fileOccup = occupiedSquares >> (squareToSquareTranspose(from) / 8) * 8;
	return _movePosBoardFile[from][fileOccup];
}

// Returns the 45 angle rotated bitboard for possible a1h8 diagonal moves from Square from. 
// The occupiedSquares bitboard is the 45 angle rotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::getLegalDiagA1h8Moves(Square from, const Bitboard& occupiedSquares) const
{
	Bitrank diagOccup = occupiedSquares >> (squareToSquareA1h8(from) / 8) * 8;
	return _movePosBoardDiagA1h8[from][diagOccup];
}

// Returns the -45 angle rotated bitboard for possible a8h1 diagonal moves from Square from. 
// The occupiedSquares bitboard is the -45 angle rotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::getLegalDiagA8h1Moves(Square from, const Bitboard& occupiedSquares) const
{
	Bitrank diagOccup = occupiedSquares >> (squareToSquareA8h1(from) / 8) * 8;
	return _movePosBoardDiagA8h1[from][diagOccup];
}

// Returns bitboard of possible knight moves from square from
Bitboard BitboardImpl::getLegalKnightMoves(Square from) const 
{
	return _movePosBoardKnight[from];
}

// Returns bitboard of possible king moves from square from
Bitboard BitboardImpl::getLegalKingMoves(Square from) const
{
	return _movePosBoardKing[from];
}

// Returns bitboard of possible white pawn attacking moves from square from
Bitboard BitboardImpl::getLegalPawnWhiteAttackingMoves(Square from) const
{
	return _attackingPosBoardPawnWhite[from - A2];
}

// Returns bitboard of possible black pawn attacking moves from square from
Bitboard BitboardImpl::getLegalPawnBlackAttackingMoves(Square from) const
{
	return _attackingPosBoardPawnBlack[from - A2];
}

// Returns the bitboard of possible white pawn positions from where it can 
// check the king at position kingPos
Bitboard BitboardImpl::getPawnWhiteCheckingPos(Square kingPos) const
{
	if (kingPos >= A4) {
		return _attackingPosBoardPawnWhite[kingPos - A4];
	}
	else if (kingPos >= A3) {
		return _attackingPosBoardPawnBlack[kingPos - A2];
	}
	else {
		return 0;
	}
}

// Returns the bitboard of possible black pawn positions from where it can
// check the king at position kingPos
Bitboard BitboardImpl::getPawnBlackCheckingPos(Square kingPos) const
{
	if (kingPos <= H5) {
		return _attackingPosBoardPawnBlack[kingPos + A2];
	}
	else if (kingPos <= H6) {
		return _attackingPosBoardPawnWhite[kingPos - A2];
	}
	else {
		return 0;
	}
}

// Returns PinInfo for the rank ray when the king is at position kingSq
// occupiedSquares is the normal bitboard of occupied squares 
const PinInfo& BitboardImpl::getRankPinInfo(Square kingSq, const Bitboard& occupiedSquares) const
{
	Bitrank rankOccup = occupiedSquares >> (kingSq / 8) * 8;
	return _rankPinInfo[kingSq][rankOccup];
}

// Returns PinInfo for the file ray when the king is at position kingSq
// occupiedSquares is the transposed bitboard of normal occupied squares 
const PinInfo& BitboardImpl::getFilePinInfo(Square kingSq, const Bitboard& occupiedSquares) const
{
	Bitrank fileOccup = occupiedSquares >> (squareToSquareTranspose(kingSq) / 8) * 8;
	return _filePinInfo[kingSq][fileOccup];
}

// Returns PinInfo for a1h8 diagonal ray when the king is at position kingSq 
// occupiedSquares is the 45 angle rotated bitboard of normal occupied squares
const PinInfo& BitboardImpl::getDiagA1h8PinInfo(Square kingSq, const Bitboard& occupiedSquares) const
{
	Bitrank diagOccup = occupiedSquares >> (squareToSquareA1h8(kingSq) / 8) * 8;
	return _diagA1h8PinInfo[kingSq][diagOccup];
}

// Returns PinInfo for a8h1 diagonal ray when the king is at position kingSq 
// occupiedSquares is the -45 angle rotated bitboard of normal occupied squares
const PinInfo& BitboardImpl::getDiagA8h1PinInfo(Square kingSq, const Bitboard& occupiedSquares) const
{
	Bitrank diagOccup = occupiedSquares >> (squareToSquareA8h1(kingSq) / 8) * 8;
	return _diagA8h1PinInfo[kingSq][diagOccup];
}

// Changes the values of leftPos and rightPos to the squares where sliding piece and the king can be located
// when enPassant capture occurs which opens check possibility
// for example, if enPassant capture is from D5 to E6 then leftPos is the closest occupied square left to D5
// rightPos is the closest occupied square right to E5. If there is no piece they are set to INVALID_SQUARE
void BitboardImpl::getEnPassantPinInfo(Square from, Square to, const Bitboard& occupiedSquares, Square& leftPos, Square& rightPos) const
{
	Bitrank rankOccup =  occupiedSquares >> (from / 8) * 8;
	assert(rankOccup & (1 << (to % 8)));
	
	unsigned int leftPin;
	unsigned int rightPin;
	if ((to % 8) > (from % 8)) {
		leftPin = to % 8;
		rightPin = from % 8;
	}
	else {
		leftPin = from % 8;
		rightPin = to % 8;
	}
	int rsbPos = findMsbSet(rankOccup << (8 - rightPin));
	int lsbPos = findLsbSet(rankOccup >> (leftPin + 1));
	leftPos = (rsbPos != -1) ? (Square) ((from / 8) * 8 + (rightPin + rsbPos - 8)) : INVALID_SQUARE;
	rightPos = (lsbPos != -1) ? (Square) ((from / 8) * 8 + (leftPin + lsbPos + 1)) : INVALID_SQUARE;
}

// Returns the bitboard of positions where sliding piece can move from square from
// when it is the only piece on the board, it should be used for finding
// possible moves which open discovered check or the movement of pinned piece
Bitboard BitboardImpl::getSlidingPieceMoves(Square from) const
{
	return _slidingPosBoard[from];
}

// Returns the board of positions where the piece with the same color as king is to move
// to block the check made by sliding piece on square from
Bitboard BitboardImpl::getSquaresBetween(Square from, Square kingSq) const
{
	return _squaresBetween[from][kingSq];
}

// Returns the converted normal bitboard from transpose bitboard
Bitboard BitboardImpl::bitboardTransposeToBitboard(const Bitboard& boardTranspose) const
{
	Bitboard tmp;
	Bitboard result = boardTranspose;
	const Bitboard mask1 = 0xF0F0F0F00F0F0F0F;
	const Bitboard mask2 = 0xCCCC0000CCCC0000;
	const Bitboard mask3 = 0xAA00AA00AA00AA00;
	tmp = result ^ (result << 36);
	result ^= mask1 & (tmp ^ (result >> 36));
	tmp = mask2 & (result ^ (result << 18));
	result ^= tmp ^ (tmp >> 18);
	tmp = mask3 & (result ^ (result << 9));
	result ^= tmp ^ (tmp >> 9);
	return result;
}

// Returns the converted normal bitboard from diagonal a1h8 bitboard
Bitboard BitboardImpl::bitboardDiagA1h8ToBitboard(const Bitboard& boardDiagA1h8) const
{
	Bitboard result = boardDiagA1h8;
	const Bitboard mask1 = 0x5555555555555555;
	const Bitboard mask2 = 0x3333333333333333;
	const Bitboard mask3 = 0x0F0F0F0F0F0F0F0F;
	result ^= mask1 & (result ^ rotateBitboardRight(result, 8));
	result ^= mask2 & (result ^ rotateBitboardRight(result, 16));
	result ^= mask3 & (result ^ rotateBitboardRight(result, 32));
	return rotateBitboardRight(result, 8);
}

// Returns the converted normal bitboard from diagonal a8h1 bitboard
Bitboard BitboardImpl::bitboardDiagA8h1ToBitboard(const Bitboard& boardDiagA8h1) const
{
	Bitboard result = boardDiagA8h1;
	const Bitboard mask1 = 0xAAAAAAAAAAAAAAAA;
	const Bitboard mask2 = 0xCCCCCCCCCCCCCCCC;
	const Bitboard mask3 = 0xF0F0F0F0F0F0F0F0;
	result ^= mask1 & (result ^ rotateBitboardRight(result, 8));
	result ^= mask2 & (result ^ rotateBitboardRight(result, 16));
	result ^= mask3 & (result ^ rotateBitboardRight(result, 32));
	return result;
}

// Normal Bitboard.                    Flipped Bitboard.
// a8 b8 c8 d8 e8 f8 g8 h8             a8 a7 a6 a5 a4 a3 a2 a1 
// a7 b7 c7 d7 e7 f7 g7 h7             b8 b7 b6 b5 b4 b3 b2 b1 
// a6 b6 c6 d6 e6 f6 g6 h6             c8 c7 c6 c5 c4 c3 c2 c1 
// a5 b5 c5 d5 e5 f5 g5 h5             d8 d7 d6 d5 d4 d3 d2 d1 
// a4 b4 c4 d4 e4 f4 g4 h4             e8 e7 e6 e5 e4 e3 e2 e1 
// a3 b3 c3 d3 e3 f3 g3 h3             f8 f7 f6 f5 f4 f3 f2 f1 
// a2 b2 c2 d2 e2 f2 g2 h2             g8 g7 g6 g5 g4 g3 g2 g1 
// a1 b1 c1 d1 e1 f1 g1 h1             h8 h7 h6 h5 h4 h3 h2 h1 
// Square to Square conversion from unrotated board to transposed board
Square BitboardImpl::squareToSquareTranspose(Square sq) const
{
	return (Square) (63 - sq / 8 - (sq % 8) * 8);
}


// Normal Bitboard.                    Flipped Bitboard.
// a8 b8 c8 d8 e8 f8 g8 h8             a8|b1 c2 d3 e4 f5 g6 h7 
// a7 b7 c7 d7 e7 f7 g7 h7             a7 b8|c1 d2 e3 f4 g5 h6
// a6 b6 c6 d6 e6 f6 g6 h6             a6 b7 c8|d1 e2 f3 g4 h5 
// a5 b5 c5 d5 e5 f5 g5 h5             a5 b6 c7 d8|e1 f2 g3 h4 
// a4 b4 c4 d4 e4 f4 g4 h4             a4 b5 c6 d7 e8|f1 g2 h3
// a3 b3 c3 d3 e3 f3 g3 h3             a3 b4 c5 d6 e7 f8|g1 h2
// a2 b2 c2 d2 e2 f2 g2 h2             a2 b3 c4 d5 e6 f7 g8|h1 
// a1 b1 c1 d1 e1 f1 g1 h1             a1 b2 c3 d4 e5 f6 g7 h8 
// Square to Square conversion from unrotated board to 45 angle rotated board
Square BitboardImpl::squareToSquareA1h8(Square sq) const
{
	int sqA1h8 = sq - (sq % 8) * 8;
	if (sqA1h8 < 0) {
		return (Square) (64 + sqA1h8);
	}
	else {
		return (Square) sqA1h8;
	}
}

// Normal Bitboard.                    Flipped Bitboard.
// a8 b8 c8 d8 e8 f8 g8 h8             a8 b7 c6 d5 e4 f3 g2 h1 
// a7 b7 c7 d7 e7 f7 g7 h7             a7 b6 c5 d4 e3 f2 g1|h8
// a6 b6 c6 d6 e6 f6 g6 h6             a6 b5 c4 d3 e2 f1|g8 h7 
// a5 b5 c5 d5 e5 f5 g5 h5             a5 b4 c3 d2 e1|f8 g7 h6
// a4 b4 c4 d4 e4 f4 g4 h4             a4 b3 c2 d1|e8 f7 g6 h5
// a3 b3 c3 d3 e3 f3 g3 h3             a3 b2 c1|d8 e7 f6 g5 h4
// a2 b2 c2 d2 e2 f2 g2 h2             a2 b1|c8 d7 e6 f5 g4 h3
// a1 b1 c1 d1 e1 f1 g1 h1             a1|b8 c7 d6 e5 f4 g3 h2
//Square to Square conversion from unrotated board to -45 angle rotated board
Square BitboardImpl::squareToSquareA8h1(Square sq) const
{
	int sqA8h1 = sq + (sq % 8) * 8;
	if (sqA8h1 > 64) {
		return (Square) (sqA8h1 - 64);
	}
	else {
		return (Square) sqA8h1;
	}
}

// Initializes squareToBitboard array for all squares
void BitboardImpl::initSquareToBitboard()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_squareToBitboard[sq] = (tmp << sq);
	}
}

// Initializes square to transposed bitboard array for all squares 
void BitboardImpl::initSquareToBitboardTranspose()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_squareToBitboardTranspose[sq] = (tmp << squareToSquareTranspose((Square) sq));
	}
}

// Initializes square to 45 degree rotated bitboard array for all squares
void BitboardImpl::initSquareToBitboardA1h8()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_squareToBitboardA1h8[sq] = (tmp << squareToSquareA1h8((Square) sq));
	}
}

// Initializes square to -45 degree rotated bitboard array for all squares
void BitboardImpl::initSquareToBitboardA8h1()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_squareToBitboardA8h1[sq] = (tmp << squareToSquareA8h1((Square) sq));
	}
}

// Initializes rank movement position bitboard array for all squares and for all available
// occupations of the rank
void BitboardImpl::initMovePosBoardRank()
{
	Bitrank rankTmp;
	Bitboard boardTmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		for (unsigned int rankOccup = 0; rankOccup < 256; ++rankOccup) {
			rankTmp = rankOccup; 
			if (rankTmp & (1 << sq % 8)) {
				rankTmp = movePosRank(sq % 8, rankTmp);
				boardTmp = rankTmp;
				boardTmp  <<= (sq / 8) * 8;
				_movePosBoardRank[sq][rankOccup] = boardTmp;
			}
			else {
				_movePosBoardRank[sq][rankOccup] = 0;
			}
		}
	}
}

// Initializes file movement position tranposed bitboard array for all squares and for 
// all available occupations of the file
void BitboardImpl::initMovePosBoardFile()
{
	Bitrank fileTmp;
	Bitboard boardTmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqTranspose = squareToSquareTranspose((Square) sq);
		for (unsigned int fileOccup = 0; fileOccup < 256; ++fileOccup) {
			fileTmp = fileOccup; 
			if (fileTmp & (1 << sqTranspose % 8)) {
				fileTmp = movePosRank(sqTranspose % 8, fileTmp);
				boardTmp = fileTmp;
				boardTmp  <<= (sqTranspose / 8) * 8;
				_movePosBoardFile[sq][fileOccup] = boardTmp;
			}
			else {
				_movePosBoardFile[sq][fileOccup] = 0;
			}
		}
	}
}

// Initializes diagonal a1h8 movement position 45 degree rotated bitboard array
// for all squares and for all available occupations of the diagonal
void BitboardImpl::initMovePosBoardA1h8()
{
	Bitrank diagA1h8Tmp;
	Bitrank diagAllowed;
	Bitboard boardTmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqA1h8 = squareToSquareA1h8((Square) sq);
		if (sqA1h8 % 8 < 8 - sqA1h8 / 8) {
			diagAllowed = OCCUPATION_FROM_LSB[8 - sqA1h8 / 8];
		}
		else {
			diagAllowed = ~OCCUPATION_FROM_LSB[8 - sqA1h8 / 8];
		}
		for (unsigned int diagOccup = 0; diagOccup < 256; ++diagOccup) {
			diagA1h8Tmp = diagOccup; 
			if (diagA1h8Tmp & (1 << sqA1h8 % 8)) {
				diagA1h8Tmp = movePosRank(sqA1h8 % 8, diagA1h8Tmp);
				boardTmp = (diagA1h8Tmp & diagAllowed);
				boardTmp  <<= (sqA1h8 / 8) * 8;
				_movePosBoardDiagA1h8[sq][diagOccup] = boardTmp;
			}
			else {
				_movePosBoardDiagA1h8[sq][diagOccup] = 0;
			}
		}
	}
}

// Initializes diagonal a8h1 movement position -45 degree rotated bitboard array
// for all squares and for all available occupations of the diagonal
void BitboardImpl::initMovePosBoardA8h1()
{
	Bitrank diagA8h1Tmp;
	Bitrank diagAllowed;
	Bitboard boardTmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqA8h1 = squareToSquareA8h1((Square) sq);
		if (sqA8h1 % 8 < sqA8h1 / 8 + 1) {
			diagAllowed = OCCUPATION_FROM_LSB[sqA8h1 / 8 + 1];
		}
		else {
			diagAllowed = ~OCCUPATION_FROM_LSB[sqA8h1 / 8 + 1];
		}
		for (unsigned int diagOccup = 0; diagOccup < 256; ++diagOccup) {
			diagA8h1Tmp = diagOccup; 
			if (diagA8h1Tmp & (1 << sqA8h1 % 8)) {
				diagA8h1Tmp = movePosRank(sqA8h1 % 8, diagA8h1Tmp);
				boardTmp = (diagA8h1Tmp & diagAllowed);
				boardTmp  <<= (sqA8h1 / 8) * 8;
				_movePosBoardDiagA8h1[sq][diagOccup] = boardTmp;
			}
			else {
				_movePosBoardDiagA8h1[sq][diagOccup] = 0;
			}
		}
	}
}

// Initializes knight movement position bitboard array for all squares
void BitboardImpl::initMovePosBoardKnight()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		switch (sq % 8) {
			case 0:
				_movePosBoardKnight[sq] = (sq <= A3 ? (KNIGHT_MOVES_A3 >> (A3 - sq)) : (KNIGHT_MOVES_A3 << (sq - A3)));
				break;
			case 1:
				_movePosBoardKnight[sq] = (sq <= B3 ? (KNIGHT_MOVES_B3 >> (B3 - sq)) : (KNIGHT_MOVES_B3 << (sq - B3)));
				break;
			case 6:
				_movePosBoardKnight[sq] = (sq <= G3 ? (KNIGHT_MOVES_G3 >> (G3 - sq)) : (KNIGHT_MOVES_G3 << (sq - G3)));
				break;
			case 7:
				_movePosBoardKnight[sq] = (sq <= H3 ? (KNIGHT_MOVES_H3 >> (H3 - sq)) : (KNIGHT_MOVES_H3 << (sq - H3)));
				break;
			default:
				_movePosBoardKnight[sq] = (sq <= C3 ? (KNIGHT_MOVES_C3 >> (C3 - sq)) : (KNIGHT_MOVES_C3 << (sq - C3)));
		}
	}
}

// Initializes king movement position bitboard array for all squares
void BitboardImpl::initMovePosBoardKing()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		switch (sq % 8) {
			case 0:
				_movePosBoardKing[sq] = (sq <= A2 ? (KING_MOVES_A2 >> (A2 - sq)) : (KING_MOVES_A2 << (sq - A2)));
				break;
			case 7:
				_movePosBoardKing[sq] = (sq <= H2 ? (KING_MOVES_H2 >> (H2 - sq)) : (KING_MOVES_H2 << (sq - H2)));
				break;
			default:
				_movePosBoardKing[sq] = (sq <= B2 ? (KING_MOVES_B2 >> (B2 - sq)) : (KING_MOVES_B2 << (sq - B2)));
		}
	}
}

// Initializes white pawn attacking position bitboard array for all squares
// where white pawn allowed to be
void BitboardImpl::initAttackingPosBoardPawnWhite()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		if (sq % 8 == 0) {
			_attackingPosBoardPawnWhite[sq - A2] = (PAWN_WHITE_ATTACK_A2 << (sq - A2));
		}
		else if (sq % 8 == 7) {
			_attackingPosBoardPawnWhite[sq - A2] = (PAWN_WHITE_ATTACK_H2 << (sq - H2));
		}
		else {
			_attackingPosBoardPawnWhite[sq - A2] = (PAWN_WHITE_ATTACK_B2 << (sq - B2));
		}
	}
}

// Initializes black pawn attacking position bitboard array for all squares
// where black pawn allowed to be
void BitboardImpl::initAttackingPosBoardPawnBlack()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		if (sq % 8 == 0) {
			_attackingPosBoardPawnBlack[sq - A2] = (PAWN_BLACK_ATTACK_A7 >> (A7 - sq));
		}
		else if (sq % 8 == 7) {
			_attackingPosBoardPawnBlack[sq - A2] = (PAWN_BLACK_ATTACK_H7 >> (H7 - sq));
		}
		else {
			_attackingPosBoardPawnBlack[sq - A2] = (PAWN_BLACK_ATTACK_G7 >> (G7 - sq));
		}
	}
}

// Initializes _rankPinInfo array for all possible rank occupations and 
// for all squares where king can be located
// PinInfo is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the leftSlide square is smaller than king square
// and rightSlide is bigger than king square
// in leftBoard and rightBoard Bitboards positions of pinned pieces are stored plus
// the positions of sliding pieces 
void BitboardImpl::initRankPinInfo()
{
	Bitrank leftRank;
	Bitrank rightRank;
	Bitboard leftBoard;
	Bitboard rightBoard;
	int leftSlidePos;
	int rightSlidePos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		for (unsigned int rankOccup = 0; rankOccup < 256; ++rankOccup) {
			if (rankOccup & (1 << sq % 8)) {
				setRankPinInfo(sq % 8, rankOccup, rightSlidePos, leftSlidePos, rightRank, leftRank);
				leftBoard = leftRank;
				rightBoard = rightRank;
				if (leftBoard) {
					leftBoard <<= (sq / 8) * 8;
				}
				if (rightBoard) {
					rightBoard <<= (sq / 8) * 8;
				}
				Square leftSlide = (leftSlidePos != -1) ? (Square) ((sq / 8) * 8 + leftSlidePos) : INVALID_SQUARE;
				Square rightSlide = (rightSlidePos != -1) ? (Square) ((sq / 8) * 8 + rightSlidePos) : INVALID_SQUARE;
				_rankPinInfo[sq][rankOccup] = PinInfo(leftSlide, rightSlide, leftBoard, rightBoard);
			}
			else {
				_rankPinInfo[sq][rankOccup] = PinInfo();
			}
		}
	}
}

// Initializes _filePinInfo array for all possible file occupations and 
// for all squares where king can be located
// PinInfo is comprised from sliding piece positions to the up and down of king position 
// in this case the square of the upSlide square is bigger than king square
// and downSlide is smaller than king square
// in upBoard and downBoard Bitboards positions of pinned pieces are stored plus the
// the positions of sliding pieces; Bitboards in the PinInfo are transposed of normal Bitboards
void BitboardImpl::initFilePinInfo()
{
	Bitrank upFile;
	Bitrank downFile;
	Bitboard upBoard;
	Bitboard downBoard;
	int upSlidePos;
	int downSlidePos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqTranspose = squareToSquareTranspose((Square) sq);
		for (unsigned int fileOccup = 0; fileOccup < 256; ++fileOccup) {
			if (fileOccup & (1 << sqTranspose % 8)) {
				setRankPinInfo(sqTranspose % 8, fileOccup, downSlidePos, upSlidePos, downFile, upFile);
				upBoard = upFile;
				downBoard = downFile;
				if (upBoard) {
					upBoard  <<= (sqTranspose / 8) * 8;
				}
				if (downBoard) {
					downBoard <<= (sqTranspose / 8) * 8;
				}
				Square upSlide = (upSlidePos != -1) ? squareToSquareTranspose((Square) ((sqTranspose / 8) * 8 + upSlidePos)) : INVALID_SQUARE;
				Square downSlide = (downSlidePos != -1) ? squareToSquareTranspose((Square) ((sqTranspose / 8) * 8 + downSlidePos)) : INVALID_SQUARE;
				_filePinInfo[sq][fileOccup] = PinInfo(downSlide, upSlide, downBoard, upBoard);
			}
			else {
				_filePinInfo[sq][fileOccup] = PinInfo();
			}
		}
	}
}

// Initializes _diagA1h8PinInfo array for all possible a1h8 diagonal occupations and 
// for all squares where king can be located
// PinInfo is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the rightSlide square is bigger than king square
// and leftSlide is smaller than king square
// in rightBoard and leftBoard Bitboards positions of the pinned pieces is stored plus
// the positions of sliding pieces; Bitboards in PinInfo are 45 degree rotated form of the normal Bitboards
void BitboardImpl::initDiagA1h8PinInfo()
{
	Bitrank leftDiag;
	Bitrank rightDiag;
	Bitrank diagAllowed;
	Bitboard leftBoard;
	Bitboard rightBoard;
	int leftSlidePos;
	int rightSlidePos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqA1h8 = squareToSquareA1h8((Square) sq);
		if (sqA1h8 % 8 < 8 - sqA1h8 / 8) {
			diagAllowed = OCCUPATION_FROM_LSB[8 - sqA1h8 / 8];
		}
		else {
			diagAllowed = ~OCCUPATION_FROM_LSB[8 - sqA1h8 / 8];
		}
		for (unsigned int diagOccup = 0; diagOccup < 256; ++diagOccup) {
			if (diagOccup & (1 << sqA1h8 % 8)) {
				setRankPinInfo(sqA1h8 % 8, diagOccup & diagAllowed, rightSlidePos, leftSlidePos, rightDiag, leftDiag);
				leftBoard = leftDiag;
				rightBoard = rightDiag;
				if (leftBoard) {
					leftBoard  <<= (sqA1h8 / 8) * 8;
				}
				if (rightBoard) {
					rightBoard <<= (sqA1h8 / 8) * 8;
				}
				Square leftSlide = (leftSlidePos != -1) ? squareToSquareA8h1((Square) ((sqA1h8 / 8) * 8 + leftSlidePos)) : INVALID_SQUARE;
				Square rightSlide = (rightSlidePos != -1) ? squareToSquareA8h1((Square) ((sqA1h8 / 8) * 8 + rightSlidePos)) : INVALID_SQUARE;
				_diagA1h8PinInfo[sq][diagOccup] = PinInfo(leftSlide, rightSlide, leftBoard, rightBoard);
			}
			else {
				_diagA1h8PinInfo[sq][diagOccup] = PinInfo();
			}
		}
	}
}

// Initializes _diagA8h1PinInfo array for all possible a8h1 diagonal occupations and 
// for all squares where king can be located
// PinInfo is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the rightSlide square is smaller than king square
// and leftSlide is bigger than king square
// in rightBoard and leftBoard Bitboards positions of the pinned pieces is stored plus
// the positions of the sliding pieces; Bitboards in PinInfo are -45 degree rotated form of the normal Bitboards
void BitboardImpl::initDiagA8h1PinInfo()
{
	Bitrank leftDiag;
	Bitrank rightDiag;
	Bitrank diagAllowed;
	Bitboard leftBoard;
	Bitboard rightBoard;
	int leftSlidePos;
	int rightSlidePos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sqA8h1 = squareToSquareA8h1((Square) sq);
		if (sqA8h1 % 8 < sqA8h1 / 8 + 1) {
			diagAllowed = OCCUPATION_FROM_LSB[sqA8h1 / 8 + 1];
		}
		else {
			diagAllowed = ~OCCUPATION_FROM_LSB[sqA8h1 / 8 + 1];
		}
		for (unsigned int diagOccup = 0; diagOccup < 256; ++diagOccup) {
			if (diagOccup & (1 << sqA8h1 % 8)) {
				setRankPinInfo(sqA8h1 % 8, diagOccup & diagAllowed, rightSlidePos, leftSlidePos, rightDiag, leftDiag);
				leftBoard = leftDiag;
				rightBoard = rightDiag;
				if (leftBoard) {
					leftBoard <<= (sqA8h1 / 8) * 8;
				}
				if (rightBoard) {
					rightBoard <<= (sqA8h1 / 8) * 8;
				}
				Square leftSlide = (leftSlidePos != -1) ? squareToSquareA1h8((Square) ((sqA8h1 / 8) * 8 + leftSlidePos)) : INVALID_SQUARE;
				Square rightSlide = (rightSlidePos != -1) ? squareToSquareA1h8((Square) ((sqA8h1 / 8) * 8 + rightSlidePos)) : INVALID_SQUARE;
				_diagA8h1PinInfo[sq][diagOccup] = PinInfo(rightSlide, leftSlide, rightBoard, leftBoard);
			}
			else {
				_diagA8h1PinInfo[sq][diagOccup] = PinInfo();
			}
		}
	}
}

// Initializes _slidingPosBoard array for each square to the Bitboard  
// of possible positions where sliding piece can move 
// from appropriate square
// This is used to identify whether move can potentially
// classify for opening discovered check or pinned piece move
void BitboardImpl::initSlidingPosBoard()
{
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Bitboard slidePos = 0;
		Square from = (Square) sq;
		slidePos |= getLegalRankMoves(from, squareToBitboard(from));
		slidePos |= bitboardTransposeToBitboard(getLegalFileMoves(from, squareToBitboardTranspose(from)));
		slidePos |= bitboardDiagA1h8ToBitboard(getLegalDiagA1h8Moves(from, squareToBitboardDiagA1h8(from)));
		slidePos |= bitboardDiagA8h1ToBitboard(getLegalDiagA8h1Moves(from, squareToBitboardDiagA8h1(from)));
		_slidingPosBoard[sq] = slidePos;
	}
}

// Initializes _squaresBetween array to bitboards of positions
// in the ray between piece and the king, plus the piece position
// For example, if the piece is on A1 and the king is on E5
// then the positions on the diagonal from A1 to E5 (excluding E5) are set
// If the piece and the king are not on the ray initializes the bitboard
// to 0
void BitboardImpl::initSquaresBetween()
{
	for (unsigned int pieceSq = A1; pieceSq < NUMBER_OF_SQUARES; ++ pieceSq) {
		for (unsigned int kingSq = A1; kingSq < NUMBER_OF_SQUARES; ++kingSq) {
			_squaresBetween[pieceSq][kingSq] = 0;
		}
	}
	PossibleMoves posMoves;

	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square pieceSq = (Square) sq;

		const std::vector<Square>& kingLeftRankPos = posMoves.possibleLeftRankMoves(pieceSq);
		for (unsigned int i = 0; i < kingLeftRankPos.size(); ++i) {
			Square kingSq = kingLeftRankPos[i];
			Square movePieceSq = (Square) (kingSq + 1);
			Bitboard pos = getLegalRankMoves(movePieceSq, squareToBitboard(pieceSq) | squareToBitboard(movePieceSq) | squareToBitboard(kingSq));
			_squaresBetween[pieceSq][kingSq] = (pos ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}
	
		const std::vector<Square>& kingRightRankPos = posMoves.possibleRightRankMoves(pieceSq);
		for (unsigned int i = 0; i < kingRightRankPos.size(); ++i) {
			Square kingSq = kingRightRankPos[i];
			Square movePieceSq = (Square) (pieceSq + 1);
			Bitboard pos = getLegalRankMoves(movePieceSq, squareToBitboard(pieceSq) | squareToBitboard(movePieceSq) | squareToBitboard(kingSq));
			_squaresBetween[pieceSq][kingSq] = (pos ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}

		const std::vector<Square>& kingUpFilePos = posMoves.possibleUpFileMoves(pieceSq);
		for (unsigned int i = 0; i < kingUpFilePos.size(); ++i) {
			Square kingSq = kingUpFilePos[i];
			Square movePieceSq = (Square) (pieceSq + 8);
			Bitboard pos = getLegalFileMoves(movePieceSq, squareToBitboardTranspose(pieceSq) | squareToBitboardTranspose(movePieceSq) | squareToBitboardTranspose(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardTransposeToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}
		
		const std::vector<Square>& kingDownFilePos = posMoves.possibleDownFileMoves(pieceSq);
		for (unsigned int i = 0; i < kingDownFilePos.size(); ++i) {
			Square kingSq = kingDownFilePos[i];
			Square movePieceSq = (Square) (kingSq + 8);
			Bitboard pos = getLegalFileMoves(movePieceSq, squareToBitboardTranspose(pieceSq) | squareToBitboardTranspose(movePieceSq) | squareToBitboardTranspose(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardTransposeToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}
		
		const std::vector<Square>& kingUpDiagA1h8Pos = posMoves.possibleUpDiagA1h8Moves(pieceSq);
		for (unsigned int i = 0; i < kingUpDiagA1h8Pos.size(); ++i) {
			Square kingSq = kingUpDiagA1h8Pos[i];
			Square movePieceSq = (Square) (pieceSq + 9);
			Bitboard pos = getLegalDiagA1h8Moves(movePieceSq, squareToBitboardDiagA1h8(pieceSq) | squareToBitboardDiagA1h8(movePieceSq) | squareToBitboardDiagA1h8(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardDiagA1h8ToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}

		const std::vector<Square>& kingDownDiagA1h8Pos = posMoves.possibleDownDiagA1h8Moves(pieceSq);
		for (unsigned int i = 0; i < kingDownDiagA1h8Pos.size(); ++i) {
			Square kingSq = kingDownDiagA1h8Pos[i];
			Square movePieceSq = (Square) (kingSq + 9);
			Bitboard pos = getLegalDiagA1h8Moves(movePieceSq, squareToBitboardDiagA1h8(pieceSq) | squareToBitboardDiagA1h8(movePieceSq) | squareToBitboardDiagA1h8(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardDiagA1h8ToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}

		const std::vector<Square>& kingUpDiagA8h1Pos = posMoves.possibleUpDiagA8h1Moves(pieceSq);
		for (unsigned int i = 0; i < kingUpDiagA8h1Pos.size(); ++i) {
			Square kingSq = kingUpDiagA8h1Pos[i];
			Square movePieceSq = (Square) (pieceSq + 7);
			Bitboard pos = getLegalDiagA8h1Moves(movePieceSq, squareToBitboardDiagA8h1(pieceSq) | squareToBitboardDiagA8h1(movePieceSq) | squareToBitboardDiagA8h1(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardDiagA8h1ToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}

		const std::vector<Square>& kingDownDiagA8h1Pos = posMoves.possibleDownDiagA8h1Moves(pieceSq);
		for (unsigned int i = 0; i < kingDownDiagA8h1Pos.size(); ++i) {
			Square kingSq = kingDownDiagA8h1Pos[i];
			Square movePieceSq = (Square) (kingSq + 7);
			Bitboard pos = getLegalDiagA8h1Moves(movePieceSq, squareToBitboardDiagA8h1(pieceSq) | squareToBitboardDiagA8h1(movePieceSq) | squareToBitboardDiagA8h1(kingSq));
			_squaresBetween[pieceSq][kingSq] = (bitboardDiagA8h1ToBitboard(pos) ^ squareToBitboard(kingSq)) | squareToBitboard(movePieceSq);
		}
	}
}


// Returns Bitrank of possible moves for rankOccup occupied bitrank when the piece is at position
// for example for the Bitrank 10101011 and the position 3 it returns 00110110
Bitrank BitboardImpl::movePosRank(unsigned int position, Bitrank rankOccup) const
{
	int rightSetBit = findMsbSet(rankOccup << (8 - position));
	int leftSetBit = findLsbSet(rankOccup >> (1 + position));
	if (rightSetBit == -1 && leftSetBit == -1) {
		return OCCUPATION_FROM_LSB[8] ^ (1 << position);
	}
	else if (rightSetBit == -1) {
		return OCCUPATION_FROM_LSB[position + 2 + leftSetBit] ^ (1 << position);
	}
	else if (leftSetBit == -1) {
		return OCCUPATION_FROM_LSB[8] ^ OCCUPATION_FROM_LSB[position - 8 + rightSetBit] ^ (1 << position);
	}
	else {
		return OCCUPATION_FROM_LSB[position - 8 + rightSetBit] ^ 
		OCCUPATION_FROM_LSB[position + 2 + leftSetBit] ^ (1 << position);	
	}
	return 0;
}

// Sets leftPin to possible positions where pinned piece can be located plus the sliding piece position
// left to the attacked piece at position
// Sets rightPin to possible positions where pinned piece can be located plus the sliding piece position
// right to the attacked piece at position 
// leftSlidePos is the position of the sliding piece to the left; -1 if not there
// rightSlidePos is the position of the sliding piece to the right; -1 if not there
// for example for the Bitrank 10101011 and position 3, the leftPin is 11110000
// leftSlidePos is 7, rightPin is 00000111, rightSlidePos is 0
void BitboardImpl::setRankPinInfo(unsigned int position, Bitrank rankOccup, int& leftSlidePos, int& rightSlidePos, Bitrank& leftPin, Bitrank& rightPin) const
{
	int rightSetBit = findMsbSet(rankOccup << (8 - position));
	int rightPinPos = (rightSetBit != -1) ? (position - 8 + rightSetBit) : -1;
	int rightNextSetBit = (rightPinPos != -1) ? findMsbSet(rankOccup << (8 - rightPinPos)) : -1;
	rightSlidePos = (rightNextSetBit != -1) ? (rightPinPos - 8 + rightNextSetBit) : -1;
	int leftSetBit = findLsbSet(rankOccup >> (1 + position));
	int leftPinPos = (leftSetBit != -1) ? (position + 1 + leftSetBit) : -1;
	int leftNextSetBit = (leftPinPos != -1) ? findLsbSet(rankOccup >> (1 + leftPinPos)) : -1;
	leftSlidePos = (leftNextSetBit != -1) ? (leftPinPos + 1 + leftNextSetBit) : -1;
	leftPin = (leftSlidePos != -1) ? OCCUPATION_FROM_LSB[leftSlidePos + 1] ^ OCCUPATION_FROM_LSB[position + 1] : 0;
	rightPin = (rightSlidePos != -1) ? OCCUPATION_FROM_LSB[position] ^ OCCUPATION_FROM_LSB[rightSlidePos] : 0;
}

// Returns the position of least significant bit set in the Bitrank
// Uses binary search algorithm
int BitboardImpl::findLsbSet(Bitrank rank) const 
{
	if (rank) {
		if (rank & 0x01) {
			return 0;
		}
		else {
			int result = 1;
			if ((rank & 0x0F) == 0) {
				rank >>= 4;
				result += 4;
			}
			if ((rank & 0x03) == 0) {
				rank >>= 2;
				result += 2;
			}
			return result - (rank & 0x01);
		}
	}

	return -1;
}

// Returns the position of the most significant bit set in the Bitrank
// Uses binary search algorithm
int BitboardImpl::findMsbSet(Bitrank rank) const
{
	if (rank) {
		if (rank & 0x80) {
			return 7;
		}
		else {
			int result = 6;
			if ((rank & 0xF0) == 0) {
				rank <<= 4;
				result -= 4;
			}
			if ((rank & 0xC0) == 0) {
				rank <<= 2;
				result -= 2;
			}
			return result + ((rank & 0x80) >> 7);
		}
	}
	
	return -1; 
}

// rotates the bits of bitboard to the right
// Here rotate is similar to the shift operation, but wraps
// over the edge bits around
Bitboard BitboardImpl::rotateBitboardRight(const Bitboard& board, unsigned int num) const
{
	return ((board >> num) | (board << (64 - num)));
}
}
