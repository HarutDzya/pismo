#ifndef BITBOARDIMPL_H_
#define BITBOARDIMPL_H_

#include "utils.h"

namespace pismo
{

typedef uint8_t Bitrank;

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

const Bitrank OCCUPATION_FROM_LSB[] =
	{0x00, 0x01, 0x03, 0x07, 0x0F, 
	0x1F, 0x3F, 0x7F, 0xFF}; 

class BitboardImpl
{
public: 
	BitboardImpl();
	
	Bitboard squareToBitboardTranspose(Square sq) const;
	Bitboard squareToBitboardDiagA1h8(Square sq) const;
	Bitboard squareToBitboardDiagA8h1(Square sq) const;
	
	Bitboard getRankMoves(Square from, const Bitboard& occupiedSquares) const;
	Bitboard getFileMoves(Square from, const Bitboard& occupiedSquares) const;
	Bitboard getDiagA1h8Moves(Square from, const Bitboard& occupiedSquares) const;
	Bitboard getDiagA8h1Moves(Square from, const Bitboard& occupiedSquares) const;

	Bitboard getKnightMoves(Square from) const;
	Bitboard getKingMoves(Square from) const;
	Bitboard getPawnWhiteAttackingMoves(Square from) const;
	Bitboard getPawnBlackAttackingMoves(Square from) const;

	Bitboard rookAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard rookAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& rookPos) const;
	Bitboard bishopAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard bishopAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& bishopPos) const;
	Bitboard queenAttackFrom(Square from, const Bitboard& occupiedSquares) const;
	Bitboard queenAttackTo(Square to, const Bitboard& occupiedSquares, const Bitboard& queenPos) const;

	Bitboard getPawnWhiteCheckingPos(Square kingPos) const;
	Bitboard getPawnBlackCheckingPos(Square kingPos) const;

	const PinInfo& getRankPinInfo(Square kingSq, const Bitboard& occupiedSquares) const;
	const PinInfo& getFilePinInfo(Square kingSq, const Bitboard& occupiedSquares) const;
	const PinInfo& getDiagA1h8PinInfo(Square kingSq, const Bitboard& occupiedSquares) const;
	const PinInfo& getDiagA8h1PinInfo(Square kingSq, const Bitboard& occupiedSquares) const;

	void getEnPassantPinInfo(Square from, Square to, const Bitboard& occupiedSquares, Square& leftPos, Square& rightPos) const;

	Bitboard getSlidingPieceMoves(Square from) const;

	Bitboard getSquaresBetween(Square from, Square kingSq) const;

	Bitboard bitboardTransposeToBitboard(const Bitboard& boardTranspose) const;
	Bitboard bitboardDiagA1h8ToBitboard(const Bitboard& boardDiagA1h8) const;
	Bitboard bitboardDiagA8h1ToBitboard(const Bitboard& boardDiagA8h1) const;

//private member functions
private:
	Square squareToSquareTranspose(Square sq) const;
	Square squareToSquareA1h8(Square sq) const;
	Square squareToSquareA8h1(Square sq) const;

	void initSquareToBitboardTranspose();
	void initSquareToBitboardA1h8();
	void initSquareToBitboardA8h1();

	void initMovePosBoardRank();
	void initMovePosBoardFile();
	void initMovePosBoardA1h8();
	void initMovePosBoardA8h1();

	void initMovePosBoardKnight();
	void initMovePosBoardKing();
	void initAttackingPosBoardPawnWhite();
	void initAttackingPosBoardPawnBlack();

	void initRankPinInfo();
	void initFilePinInfo();
	void initDiagA1h8PinInfo();
	void initDiagA8h1PinInfo();

	void initSlidingPosBoard();

	void initSquaresBetween();

	Bitrank movePosRank(unsigned int position, Bitrank rankOccup) const;
	void setRankPinInfo(unsigned int position, Bitrank rankOccup, int& leftSlidePos, int& rightSlidePos, Bitrank & leftPin, Bitrank& rightPin) const;
	int findLsbSet(Bitrank rank) const;
	int findMsbSet(Bitrank rank) const;
	
	Bitboard rotateBitboardRight(const Bitboard& board, unsigned int num) const;

// date members
private:

	Bitboard _squareToBitboardTranspose[NUMBER_OF_SQUARES];
	Bitboard _squareToBitboardA1h8[NUMBER_OF_SQUARES];
	Bitboard _squareToBitboardA8h1[NUMBER_OF_SQUARES];

	// TODO: For optimization purposes change arrays to [64][64]	
	Bitboard _movePosBoardRank[NUMBER_OF_SQUARES][256];
	Bitboard _movePosBoardFile[NUMBER_OF_SQUARES][256];
	Bitboard _movePosBoardDiagA1h8[NUMBER_OF_SQUARES][256];
	Bitboard _movePosBoardDiagA8h1[NUMBER_OF_SQUARES][256];

	Bitboard _movePosBoardKnight[NUMBER_OF_SQUARES];
	Bitboard _movePosBoardKing[NUMBER_OF_SQUARES];
	Bitboard _attackingPosBoardPawnWhite[NUMBER_OF_SQUARES - 16];
	Bitboard _attackingPosBoardPawnBlack[NUMBER_OF_SQUARES - 16];

	PinInfo _rankPinInfo[NUMBER_OF_SQUARES][256];
	PinInfo _filePinInfo[NUMBER_OF_SQUARES][256];
	PinInfo _diagA1h8PinInfo[NUMBER_OF_SQUARES][256];
	PinInfo _diagA8h1PinInfo[NUMBER_OF_SQUARES][256];
	
	Bitboard _slidingPosBoard[NUMBER_OF_SQUARES];

	Bitboard _squaresBetween[NUMBER_OF_SQUARES][NUMBER_OF_SQUARES];
};
}

#endif
