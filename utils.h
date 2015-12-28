#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <string>

#define mRank(square) ((square) >> 3)
#define mFile(square) ((square) & 0x7)

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
	NUMBER_OF_SQUARES, // = 64
	INVALID_SQUARE
};


enum MoveType {
	NORMAL_MOVE = 0, CAPTURE_MOVE, PROMOTION_MOVE, CASTLING_MOVE,
        EN_PASSANT_MOVE, EN_PASSANT_CAPTURE
};

enum MoveGenerationStage {
	CAPTURING_MOVES = 0, CHECKING_MOVES,
	QUITE_MOVES, EVASION_MOVES, SEARCH_FINISHED
}; //TODO: Later add KILLER_MOVES

enum SearchType {
	USUAL_SEARCH = 0,
	EVASION_SEARCH,
	QUIESCENCE_SEARCH
};

struct MoveInfo {
	Square from;
	Square to;
	Piece promoted;
	MoveType type;
	uint16_t value;
	MoveInfo(Square f = INVALID_SQUARE, Square t = INVALID_SQUARE, Piece p = ETY_SQUARE, MoveType mt = NORMAL_MOVE, uint16_t v = 0)
		: from(f)
		  , to(t)
		  , promoted(p)
		  , type(mt)
		  , value(v)
	{
	}
};

const MoveInfo MATE_MOVE(INVALID_SQUARE, INVALID_SQUARE, ETY_SQUARE);

struct EvalInfo
{
	int16_t posValue;
	ZobKey zobKey;
	uint16_t depth;
	
	EvalInfo(int16_t v = 0, ZobKey z = 0, uint16_t d = 0)
  	: posValue(v),
    	zobKey(z),
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

std::string moveToNotation(const MoveInfo& move);
}

#endif //UTILS_H_
