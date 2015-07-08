#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

namespace pismo
{
typedef uint64_t Bitboard;
typedef uint64_t ZobKey;
 
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

struct eval_info
{
	int16_t pos_value;
	ZobKey zob_key;
	uint16_t depth;
	
	eval_info(int16_t v = 0, ZobKey z = 0, uint16_t d = 0)
  	: pos_value(v),
    	zob_key(z),
    	depth(d)
  	{
  	}

	//TODO:Add later best move
};

// Material Piece values according to enum Piece 
const int PIECE_VALUES[PIECE_NB] = 
	{100,  320,  330,  500,  900,  20000,
	-100, -320, -330, -500, -900, -20000};

const int16_t MAX_SCORE = 10000; //white has 100% winning position (-MAX_SCORE black wins)

}

#endif //UTILS_H_
