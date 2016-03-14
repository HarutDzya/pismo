#include "BitboardImpl.h"
#include "MagicMoves.h"
#include <assert.h>

namespace pismo
{
Bitboard squareToBitboard[SQUARES_COUNT] =
{
	1ULL << 0,  1ULL << 1,  1ULL << 2,  1ULL << 3,  1ULL << 4,  1ULL << 5,  1ULL << 6,  1ULL << 7,
	1ULL << 8,  1ULL << 9,  1ULL << 10, 1ULL << 11, 1ULL << 12, 1ULL << 13, 1ULL << 14, 1ULL << 15,
	1ULL << 16, 1ULL << 17, 1ULL << 18, 1ULL << 19, 1ULL << 20, 1ULL << 21, 1ULL << 22, 1ULL << 23,
	1ULL << 24, 1ULL << 25, 1ULL << 26, 1ULL << 27, 1ULL << 28, 1ULL << 29, 1ULL << 30, 1ULL << 31,
	1ULL << 32, 1ULL << 33, 1ULL << 34, 1ULL << 35, 1ULL << 36, 1ULL << 37, 1ULL << 38, 1ULL << 39,
	1ULL << 40, 1ULL << 41, 1ULL << 42, 1ULL << 43, 1ULL << 44, 1ULL << 45, 1ULL << 46, 1ULL << 47,
	1ULL << 48, 1ULL << 49, 1ULL << 50, 1ULL << 51, 1ULL << 52, 1ULL << 53, 1ULL << 54, 1ULL << 55,
	1ULL << 56, 1ULL << 57, 1ULL << 58, 1ULL << 59, 1ULL << 60, 1ULL << 61, 1ULL << 62, 1ULL << 63
};


/*
E.g. for square E6 it returns following bitboard

00001000
00001000
11110111
00001000
00001000
00001000
00001000
00001000
*/

Bitboard RankFileMask[SQUARES_COUNT] =
{
    0x1010101010101fe, 0x2020202020202fd, 0x4040404040404fb, 0x8080808080808f7, 0x10101010101010ef, 0x20202020202020df, 0x40404040404040bf, 0x808080808080807f,
    0x10101010101fe01, 0x20202020202fd02, 0x40404040404fb04, 0x80808080808f708, 0x101010101010ef10, 0x202020202020df20, 0x404040404040bf40, 0x8080808080807f80,
    0x101010101fe0101, 0x202020202fd0202, 0x404040404fb0404, 0x808080808f70808, 0x1010101010ef1010, 0x2020202020df2020, 0x4040404040bf4040, 0x80808080807f8080,
    0x1010101fe010101, 0x2020202fd020202, 0x4040404fb040404, 0x8080808f7080808, 0x10101010ef101010, 0x20202020df202020, 0x40404040bf404040, 0x808080807f808080,
    0x10101fe01010101, 0x20202fd02020202, 0x40404fb04040404, 0x80808f708080808, 0x101010ef10101010, 0x202020df20202020, 0x404040bf40404040, 0x8080807f80808080,
    0x101fe0101010101, 0x202fd0202020202, 0x404fb0404040404, 0x808f70808080808, 0x1010ef1010101010, 0x2020df2020202020, 0x4040bf4040404040, 0x80807f8080808080,
    0x1fe010101010101, 0x2fd020202020202, 0x4fb040404040404, 0x8f7080808080808, 0x10ef101010101010, 0x20df202020202020, 0x40bf404040404040, 0x807f808080808080,
    0xfe01010101010101, 0xfd02020202020202, 0xfb04040404040404, 0xf708080808080808, 0xef10101010101010, 0xdf20202020202020, 0xbf40404040404040, 0x7f80808080808080
};

/*
For square B6 it returs following bitboard

00010000
10100000
00000000
10100000
00010000
00001000
00000100
00000010

*/

Bitboard DiagonalMask[SQUARES_COUNT] =
{
    0x8040201008040200, 0x80402010080500, 0x804020110a00, 0x8041221400, 0x182442800, 0x10204885000, 0x102040810a000, 0x102040810204000,
    0x4020100804020002, 0x8040201008050005, 0x804020110a000a, 0x804122140014, 0x18244280028, 0x1020488500050, 0x102040810a000a0, 0x204081020400040,
    0x2010080402000204, 0x4020100805000508, 0x804020110a000a11, 0x80412214001422, 0x1824428002844, 0x102048850005088, 0x2040810a000a010, 0x408102040004020,
    0x1008040200020408, 0x2010080500050810, 0x4020110a000a1120, 0x8041221400142241, 0x182442800284482, 0x204885000508804, 0x40810a000a01008, 0x810204000402010,
    0x804020002040810, 0x1008050005081020, 0x20110a000a112040, 0x4122140014224180, 0x8244280028448201, 0x488500050880402, 0x810a000a0100804, 0x1020400040201008,
    0x402000204081020, 0x805000508102040, 0x110a000a11204080, 0x2214001422418000, 0x4428002844820100, 0x8850005088040201, 0x10a000a010080402, 0x2040004020100804,
    0x200020408102040, 0x500050810204080, 0xa000a1120408000, 0x1400142241800000, 0x2800284482010000, 0x5000508804020100, 0xa000a01008040201, 0x4000402010080402,
    0x2040810204080, 0x5081020408000, 0xa112040800000, 0x14224180000000, 0x28448201000000, 0x50880402010000, 0xa0100804020100, 0x40201008040201
};

const Bitboard FileABitboard = 0x0101010101010101;

Bitboard FileBitboard[FILES_COUNT] =
{
    FileABitboard << FILE_A, FileABitboard << FILE_B, FileABitboard << FILE_C, FileABitboard << FILE_D,
    FileABitboard << FILE_E, FileABitboard << FILE_F, FileABitboard << FILE_G, FileABitboard << FILE_H
};

const Bitboard Rank1Bitboard = 0x00000000000000FF;

Bitboard RankBitboard[RANKS_COUNT] =
{
    Rank1Bitboard << RANK_1 * 8, Rank1Bitboard << RANK_2 * 8, Rank1Bitboard << RANK_3 * 8, Rank1Bitboard << RANK_4 * 8,
    Rank1Bitboard << RANK_5 * 8, Rank1Bitboard << RANK_6 * 8, Rank1Bitboard << RANK_7 * 8, Rank1Bitboard << RANK_8 * 8,
};

Bitboard KingZone[SQUARES_COUNT];


const Bitboard BITSCAN_MAGIC = 0x07EDD5E59A4E28C2;

BitboardImpl* BitboardImpl::_instance = 0;

const BitboardImpl* BitboardImpl::instance()
{
	if(!_instance) {
		_instance = new BitboardImpl();
	}

	return _instance;
}

void BitboardImpl::destroy()
{
	delete _instance;
	_instance  = 0;
}

BitboardImpl::BitboardImpl()
{
	initmagicmoves();
	initBitScanTable();

	initMovePosBoardKnight();
	initMovePosBoardKing();
	initAttackingPosBoardPawnWhite();
	initAttackingPosBoardPawnBlack();

	initKingZone();

	initSquaresBetween();
}

// Returns bitboard of possible knight moves from square from
Bitboard BitboardImpl::knightAttackFrom(Square from) const 
{
	return _movePosBoardKnight[from];
}

// Returns bitboard of positions of the knights which attack the square to
// knightsPos shows all the knight positions of appropriate color
Bitboard BitboardImpl::knightsAttackTo(Square to, const Bitboard& knightsPos) const
{
	return _movePosBoardKnight[to] & knightsPos;
}

// Returns bitboard of possible king moves from square from
Bitboard BitboardImpl::kingAttackFrom(Square from) const
{
	return _movePosBoardKing[from];
}

// Returns bitboard of position of the king which attacks the square to
// kingPos shows the king positions of appropriate color
Bitboard BitboardImpl::kingAttackTo(Square to, const Bitboard& kingPos) const
{
	return _movePosBoardKing[to] & kingPos;
}

// Returns bitboard of possible white pawn attacking moves from square from
Bitboard BitboardImpl::pawnWhiteAttackFrom(Square from) const
{
	return _attackingPosBoardPawnWhite[from - A2];
}

// Returns the bitboard of possible white pawn positions from where it can 
// attack the position to
Bitboard BitboardImpl::pawnsWhiteAttackTo(Square to) const
{
	if (to >= A4) {
		return _attackingPosBoardPawnWhite[to - A4];
	}
	else if (to >= A3) {
		return _attackingPosBoardPawnBlack[to - A2];
	}
	else {
		return 0;
	}
}

// Returns the bitboard of white pawn positions which attack the square to 
// pawnsWhitePos shows white pawn positions
Bitboard BitboardImpl::pawnsWhiteAttackTo(Square to, const Bitboard& pawnsWhitePos) const
{
	if (to >= A4) {
		return _attackingPosBoardPawnWhite[to - A4] & pawnsWhitePos;
	}
	else if (to >= A3) {
		return _attackingPosBoardPawnBlack[to - A2] & pawnsWhitePos;
	}
	else {
		return 0;
	}
}

// Returns bitboard of possible black pawn attacking moves from square from
Bitboard BitboardImpl::pawnBlackAttackFrom(Square from) const
{
	return _attackingPosBoardPawnBlack[from - A2];
}

// Returns the bitboard of possible black pawn positions from where it can
// attack the position to
Bitboard BitboardImpl::pawnsBlackAttackTo(Square to) const
{
	if (to <= H5) {
		return _attackingPosBoardPawnBlack[to + A2];
	}
	else if (to <= H6) {
		return _attackingPosBoardPawnWhite[to - A2];
	}
	else {
		return 0;
	}
}

// Returns the bitboard of black pawn positions which attack the square to 
// pawnsBlackPos shows black pawn positions
Bitboard BitboardImpl::pawnsBlackAttackTo(Square to, const Bitboard& pawnsBlackPos) const
{
	if (to <= H5) {
		return _attackingPosBoardPawnBlack[to + A2] & pawnsBlackPos;
	}
	else if (to <= H6) {
		return _attackingPosBoardPawnWhite[to - A2] & pawnsBlackPos;
	}
	else {
		return 0;
	}
}

// Returns the bitboard of white pawn vertical moves from square from
// if the occupiedSquares are occupied
Bitboard BitboardImpl::pawnWhiteMovesFrom(Square from, const Bitboard& occupiedSquares) const
{
	if (from >= A2 && from <= H2) {
		if (squareToBitboard[from + 8] & ~occupiedSquares) {
			return (squareToBitboard[from + 8] | squareToBitboard[from + 16]) & ~occupiedSquares;
		}
	}
	return squareToBitboard[from + 8] & ~occupiedSquares;
}

// Returns the bitboard of black pawn vertical moves from square from
// if the occupiedSquares are occupied
Bitboard BitboardImpl::pawnBlackMovesFrom(Square from, const Bitboard& occupiedSquares) const
{
	if (from >= A7 && from <= H7) {
		if (squareToBitboard[from - 8] & ~occupiedSquares) {
			return (squareToBitboard[from - 8] | squareToBitboard[from - 16]) & ~occupiedSquares;
		}
	}
	return squareToBitboard[from - 8] & ~occupiedSquares;
}

// Returns the bitboard of white pawn vertical moves to square to
// for occupiedSquares and pawnsWhitePos white pawns positions
Bitboard BitboardImpl::pawnWhiteMovesTo(Square to, const Bitboard& occupiedSquares, const Bitboard& pawnsWhitePos) const
{
	if (to >= A4 && to <= H4) {
		if (squareToBitboard[to - 8] & ~occupiedSquares) {
			return squareToBitboard[to - 16] & pawnsWhitePos;
		}
	}
	if (to > H2) {
		return squareToBitboard[to - 8] & pawnsWhitePos;
	}
	else {
		return 0;
	}
}

// Returns the bitboard of black pawn vertical moves to square to
// for occupiedSquares and pawnsBlackPos black pawns positions
Bitboard BitboardImpl::pawnBlackMovesTo(Square to, const Bitboard& occupiedSquares, const Bitboard& pawnsBlackPos) const
{
	if (to >= A5 && to <= H5) {
		if (squareToBitboard[to + 8] & ~occupiedSquares) {
			return squareToBitboard[to + 16] & pawnsBlackPos;
		}
	}

	if (to < A7) {
	   	return squareToBitboard[to + 8] & pawnsBlackPos;
	}
	else {
		return 0;
	}
}

// Returns bitboard of possible rook moves from square from when 
// occupiedSquares shows the occupancy. Uses magic bitboards for evaluation.
Bitboard BitboardImpl::rookAttackFrom(Square from, const Bitboard& occupiedSquares) const
{
	return Rmagic(from, occupiedSquares);
}

// Returns bitboard of positions of the rooks which attack the square to
// occupiedSquares shows the occupancy, rooksPos shows all the rook positions of appropriate color 
Bitboard BitboardImpl::rooksAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& rooksPos) const
{
	return Rmagic(to, occupiedSquares) & rooksPos;
}

// Returns bitboard of possible bishop moves from square from when 
// occupiedSquares shows the occupancy. Uses magic bitboards for evaluation.
Bitboard BitboardImpl::bishopAttackFrom(Square from, const Bitboard& occupiedSquares) const
{
	return Bmagic(from, occupiedSquares);
}

// Returns bitboard of positions of the bishops which attack the square to
// occupiedSquares shows the occupancy, bishopsPos shows all the bishop positions of appropriate color 
Bitboard BitboardImpl::bishopsAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& bishopsPos) const
{
	return Bmagic(to, occupiedSquares) & bishopsPos;
}

// Returns bitboard of possible queen moves from square from when 
// occupiedSquares shows the occupancy. Uses magic bitboards for evluation.
Bitboard BitboardImpl::queenAttackFrom(Square from, const Bitboard& occupiedSquares) const
{
	return Qmagic(from, occupiedSquares);
}

// Returns bitboard of positions of the queens which attack the square to
// occupiedSquares shows the occupancy, queensPos shows all the queen positions of appropriate color 
Bitboard BitboardImpl::queensAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& queensPos) const
{
	return Qmagic(to, occupiedSquares) & queensPos;
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
	int rsbPos = msb(rankOccup << (8 - rightPin));
	int lsbPos = lsb(rankOccup >> (leftPin + 1));
	leftPos = (rsbPos != -1) ? (Square) ((from / 8) * 8 + (rightPin + rsbPos - 8)) : INVALID_SQUARE;
	rightPos = (lsbPos != -1) ? (Square) ((from / 8) * 8 + (leftPin + lsbPos + 1)) : INVALID_SQUARE;
}

// Returns the board of positions where the piece with the same color as king is to move
// to block the check made by sliding piece on square from
Bitboard BitboardImpl::getSquaresBetween(Square from, Square kingSq) const
{
	return _squaresBetween[from][kingSq];
}

// Returns the least significant set bit position in the board
// -1 if board is 0
int BitboardImpl::lsb(const Bitboard& board) const
{
	if (board) {
		return _bitScanTable[((board & (-board)) * BITSCAN_MAGIC) >> 58];
	}
	
	return -1;
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

//king zone are squares adjacent to given square
// e.g. for f5 square, kingzone are e4, f4, g4, e5, g5, e6, f6, f7 and f5
// or for a8 square - a7, b7, b8 and a8
// TODO: init directly with numbers (do not calculate)

void BitboardImpl::initKingZone()
{
  for (unsigned int sq = A1; sq < SQUARES_COUNT; ++sq)
  {
    Bitboard b = 0;
    b |= (1LU << sq);
    if (sq > 8 && sq % 8) b |= (1LU << (sq - 9));
    if (sq > 7) b |= (1LU << (sq - 8));
    if (sq > 7 && ((sq + 1) % 8)) b |= (1LU << (sq - 7));

    if (sq > 0 && sq % 8) b |= (1LU << (sq - 1));
    if (sq < 63 && ((sq + 1) % 8)) b |= (1LU << (sq + 1));

    if (sq > 0 && sq % 8 && sq < 56 ) b |= (1LU << (sq + 7));
    if (sq < 56) b |= (1LU << (sq + 8));
    if (sq < 55 && ((sq + 1) % 8)) b |= (1LU << (sq + 9));

    KingZone[sq] = b;
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
	for (unsigned int pieceSq = A1; pieceSq < SQUARES_COUNT; ++ pieceSq) {
		for (unsigned int kingSq = A1; kingSq < SQUARES_COUNT; ++kingSq) {
			_squaresBetween[pieceSq][kingSq] = 0;
		}
	}

	for (unsigned int pieceSq = A1; pieceSq < SQUARES_COUNT; ++pieceSq) {
		Bitboard kingRankFilePos = RankFileMask[pieceSq];
		while (kingRankFilePos) {
			unsigned int kingSq = lsb(kingRankFilePos);
			_squaresBetween[pieceSq][kingSq] = (rookAttackFrom((Square) pieceSq, squareToBitboard[pieceSq] | squareToBitboard[kingSq]) &
				rookAttackFrom((Square) kingSq, squareToBitboard[pieceSq] | squareToBitboard[kingSq])) | squareToBitboard[pieceSq];
			kingRankFilePos &= (kingRankFilePos - 1);
		}

		Bitboard kingDiagPos = DiagonalMask[pieceSq];
		while (kingDiagPos) {
			unsigned int kingSq = lsb(kingDiagPos);
			_squaresBetween[pieceSq][kingSq] = (bishopAttackFrom((Square) pieceSq, squareToBitboard[pieceSq] | squareToBitboard[kingSq]) &
					bishopAttackFrom((Square) kingSq, squareToBitboard[pieceSq] | squareToBitboard[kingSq])) | squareToBitboard[pieceSq];
			kingDiagPos &= (kingDiagPos - 1);
		}
	}
}

// Initializes _bitscanTable array to the position of a lsb
// for each case of the board when one bit is set to 1
// Uses magic number to compute the index
void BitboardImpl::initBitScanTable()
{
	for (unsigned int bitPos = 0; bitPos < SQUARES_COUNT; ++bitPos) {
		Bitboard board = 1ULL << bitPos;
		_bitScanTable[(board * BITSCAN_MAGIC) >> 58] = bitPos;
	}
}

// Returns the position of the most significant bit set in the Bitrank
// Uses binary search algorithm
int BitboardImpl::msb(Bitrank rank) const
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

BitboardImpl::~BitboardImpl()
{
}
}
