#ifndef BITBOARDIMPL_H_
#define BITBOARDIMPL_H_

#include "utils.h"

namespace pismo
{

typedef uint8_t Bitrank;

const Bitboard WHITE_SQUARES_MASK = 0x55AA55AA55AA55AA; // Bitboard of white squares
const Bitboard BLACK_SQUARES_MASK = 0xAA55AA55AA55AA55; // Bitboard of black squares

const Bitboard WHITE_RIGHT_CASTLING_KING_SQUARES = 0x0000000000000070; // Bitboard of squares used by the king making right castling move for white
const Bitboard WHITE_RIGHT_CASTLING_ETY_SQUARES = 0x0000000000000060; // Bitboard of squares which should be empty for right castling move for white
const Bitboard WHITE_LEFT_CASTLING_KING_SQUARES = 0x000000000000001C; // Bitboard of squares used by the king making left castling move for white
const Bitboard WHITE_LEFT_CASTLING_ETY_SQUARES = 0x000000000000000E; // Bitboard of squares which should be empty for left castling move for white
const Bitboard BLACK_RIGHT_CASTLING_KING_SQUARES = 0x7000000000000000; // Bitboard of squares used by the king making right castling move for black
const Bitboard BLACK_RIGHT_CASTLING_ETY_SQUARES = 0x6000000000000000; // Bitboard of squares which should be empty for right castling move for black
const Bitboard BLACK_LEFT_CASTLING_KING_SQUARES = 0x1C00000000000000; // Bitboard of squares used by the king making left castling move for black
const Bitboard BLACK_LEFT_CASTLING_ETY_SQUARES = 0x0E00000000000000; // Bitboard of squares which should be empty for left castling move for black

const Bitboard PAWN_WHITE_INIT = 0x000000000000FF00; // Bitboard of white pawn initial position
const Bitboard PAWN_BLACK_INIT = 0x00FF000000000000; // Bitboard of black pawn initial position

const Bitboard PAWN_WHITE_ATTACK_A2 = 0x0000000000020000; // Bitboard of white pawn attacked squares when on square A1
const Bitboard PAWN_WHITE_ATTACK_B2 = 0x0000000000050000; // Bitboard of white pawn attacked squares when on square B1
const Bitboard PAWN_WHITE_ATTACK_H2 = 0x0000000000400000; // Bitboard of white pawn attacked squares when on square H1
const Bitboard PAWN_BLACK_ATTACK_A7 = 0x0000020000000000; // Bitboard of black pawn attacked squares when on square A7
const Bitboard PAWN_BLACK_ATTACK_G7 = 0x0000A00000000000; // Bitboard of black pawn attacked squares when on square B7
const Bitboard PAWN_BLACK_ATTACK_H7 = 0x0000400000000000; // Bitboard of black pawn attacked squares when on square H7

const Bitboard KNIGHT_MOVES_C3 = 0x0000000A1100110A; // Bitboard for possible moves of knight at Square C3
const Bitboard KNIGHT_MOVES_B3 = 0x0000000508000805; // Bitboard for possible moves of knight at Square B3
const Bitboard KNIGHT_MOVES_A3 = 0x0000000204000402; // Bitboard for possible moves of knight at Square A3
const Bitboard KNIGHT_MOVES_G3 = 0x000000A0100010A0; // Bitboard for possible moves of knight at Sqaure G3
const Bitboard KNIGHT_MOVES_H3 = 0x0000004020002040; // Bitboard for possible moves of knight at Square H3

const Bitboard KING_MOVES_B2 = 0x0000000000070507; // Bitboard for possible moves of king at Square B2
const Bitboard KING_MOVES_A2 = 0x0000000000030203; // Bitboard for possible moves of king at Square A2
const Bitboard KING_MOVES_H2 = 0x0000000000C040C0; // Bitboard for possible moves of king at Square H2 

extern Bitboard squareToBitboard[SQUARES_COUNT];
extern Bitboard RankFileMask[SQUARES_COUNT];
extern Bitboard DiagonalMask[SQUARES_COUNT];

extern Bitboard FileBitboard[FILES_COUNT];
extern Bitboard RankBitboard[RANKS_COUNT];


class BitboardImpl
{
public: 
	static const BitboardImpl* instance();
	void destroy();
	
	Bitboard knightAttackFrom(Square from) const;
	Bitboard knightsAttackTo(Square to, const Bitboard& knightsPos) const;
	Bitboard kingAttackFrom(Square from) const;
	Bitboard kingAttackTo(Square to, const Bitboard& kingPos) const;
	Bitboard pawnWhiteAttackFrom(Square from) const;
	Bitboard pawnsWhiteAttackTo(Square to) const;
	Bitboard pawnsWhiteAttackTo(Square to, const Bitboard& pawnsWhitePos) const;
	Bitboard pawnBlackAttackFrom(Square from) const;
	Bitboard pawnsBlackAttackTo(Square to) const;
	Bitboard pawnsBlackAttackTo(Square to, const Bitboard& pawnsBlackPos) const;
	Bitboard rookAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard rooksAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& rooksPos) const;
	Bitboard bishopAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard bishopsAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& bishopsPos) const;
	Bitboard queenAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard queensAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& queensPos) const;
	Bitboard pawnWhiteMovesFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard pawnBlackMovesFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard pawnWhiteMovesTo(Square to, const Bitboard& occupiedSquares, const Bitboard& pawnsWhitePos) const;
	Bitboard pawnBlackMovesTo(Square to, const Bitboard& occupiedSquares, const Bitboard& pawnsBlackPos) const;

	//https://chessprogramming.wikispaces.com/Pawn+Attacks+(Bitboards)

	Bitboard shiftNorthEast(Bitboard pawns) const
	{
	  return (pawns << 9) & ~FileBitboard[FILE_A];
	}

	Bitboard shiftNorthWest(Bitboard pawns) const
	{
		return (pawns << 7) & ~FileBitboard[FILE_H];
	}
	
	Bitboard shiftSouthEast(Bitboard pawns) const
	{
		return (pawns >> 7) & ~FileBitboard[FILE_A];
	}
	
	Bitboard shiftSouthWest(Bitboard pawns) const
	{
		return (pawns >> 9) & ~FileBitboard[FILE_H];
	}

	Bitboard whitePawnEastAttacks(Bitboard whitePawns) const {return shiftNorthEast(whitePawns);}
	Bitboard whitePawnWestAttacks(Bitboard whitePawns) const {return shiftNorthWest(whitePawns);}

	Bitboard blackPawnEastAttacks(Bitboard bpawns) const {return shiftSouthEast(bpawns);}
	Bitboard blackPawnWestAttacks(Bitboard bpawns) const {return shiftSouthWest(bpawns);}

	Bitboard whitePawnAnyAttacks(Bitboard whitePawns) const
	{
		return whitePawnEastAttacks(whitePawns) | whitePawnWestAttacks(whitePawns);
	}

	Bitboard whitePawnDblAttacks(Bitboard whitePawns) const
	{
		return whitePawnEastAttacks(whitePawns) & whitePawnWestAttacks(whitePawns);
	}

	Bitboard whitePawnSingleAttacks(Bitboard whitePawns) const
	{
		return whitePawnEastAttacks(whitePawns) ^ whitePawnWestAttacks(whitePawns);
	}

	Bitboard blackPawnAnyAttacks(Bitboard blackPawns) const
	{
		return blackPawnEastAttacks(blackPawns) | blackPawnWestAttacks(blackPawns);
	}

	Bitboard blackPawnDblAttacks(Bitboard blackPawns) const
	{
		return blackPawnEastAttacks(blackPawns) & blackPawnWestAttacks(blackPawns);
	}
	
	Bitboard blackPawnSingleAttacks(Bitboard blackPawns) const
	{
		return blackPawnEastAttacks(blackPawns) ^ blackPawnWestAttacks(blackPawns);
	}



	void getEnPassantPinInfo(Square from, Square to, const Bitboard& occupiedSquares, Square& leftPos, Square& rightPos) const;

	Bitboard getSquaresBetween(Square from, Square kingSq) const;

	int lsb(const Bitboard& board) const;

//private member functions
private:
	BitboardImpl();
	~BitboardImpl();

	BitboardImpl(const BitboardImpl&); //non-copyable
	BitboardImpl& operator=(const BitboardImpl&); //non-assignable

	void initMovePosBoardKnight();
	void initMovePosBoardKing();
	void initAttackingPosBoardPawnWhite();
	void initAttackingPosBoardPawnBlack();

	void initSquaresBetween();

	void initBitScanTable();

	int msb(Bitrank rank) const;

// date members
private:
	static BitboardImpl* _instance;

	Bitboard _movePosBoardKnight[SQUARES_COUNT];
	Bitboard _movePosBoardKing[SQUARES_COUNT];
	Bitboard _attackingPosBoardPawnWhite[SQUARES_COUNT - 16];
	Bitboard _attackingPosBoardPawnBlack[SQUARES_COUNT - 16];

	Bitboard _squaresBetween[SQUARES_COUNT][SQUARES_COUNT];

	unsigned int _bitScanTable[SQUARES_COUNT];
};
}

#endif
