#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

namespace pismo
{

typedef uint64_t Bitboard;
typedef unsigned int Count;
 
enum Color { 
	WHITE = 0, 
	BLACK
};

enum Piece {
	PAWN_WHITE = 0, KNIGHT_WHITE, BISHOP_WHITE, ROOK_WHITE, 
	QUEEN_WHITE, KING_WHITE, PAWN_BLACK, KNIGHT_BLACK,
	BISHOP_BLACK, ROOK_BLACK, QUEEN_BLACK, KING_BLACK,
	PIECE_NB = 12, ETY_SQUARE = 13
};

enum Square {
	A1 = 0, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NUMBER_OF_SQUARES // = 64
};

struct move_info {
	Square from;
	Square to;
	Piece promoted;
};

const Bitboard WHITE_RIGHT_CASTLING = 0x00000000000000F0; // Bitboard of squares involved in right castling of white
const Bitboard WHITE_LEFT_CASTLING = 0x000000000000001D; // Bitboard of squares involved in left castling of white
const Bitboard BLACK_RIGHT_CASTLING = 0xF000000000000000; // Bitboard of squares involved in right castling of black
const Bitboard BLACK_LEFT_CASTLING =  0x1D00000000000000; // Bitboard of squares involved in left castling of black 

const Bitboard PAWN_WHITE_INIT = 0x000000000000FF00; // Bitboard of white pawn initial position
const Bitboard PAWN_BLACK_INIT = 0x00FF000000000000; // Bitboard of black pawn initial position


const Bitboard KNIGHT_MOVES_C3 = 0x0000000A1100110A; // Bitboard for possible moves of knight at Square C3
const Bitboard KNIGHT_MOVES_B3 = 0x0000000508000805; // Bitboard for possible moves of knight at Square B3
const Bitboard KNIGHT_MOVES_A3 = 0x0000000204000402; // Bitboard for possible moves of knight at Square A3
const Bitboard KNIGHT_MOVES_G3 = 0x000000A0100010A0; // Bitboard for possible moves of knight at Sqaure G3
const Bitboard KNIGHT_MOVES_H3 = 0x0000004020002040; // Bitboard for possible moves of knight at Square H3

const Bitboard KING_MOVES_B2 = 0x0000000000070507; // Bitboard for possible moves of king at Square B2
const Bitboard KING_MOVES_A2 = 0x0000000000030203; // Bitboard for possible moves of king at Square A2
const Bitboard KING_MOVES_H2 = 0x0000000000C080C0; // Bitboard for possible moves of king at Square H2 
}

#endif //UTILS_H_
