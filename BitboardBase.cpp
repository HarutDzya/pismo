#include "BitboardBase.h"

namespace pismo
{
Bitboard BitboardBase::square_to_bitboard(Square sq) const
{
	return _square_to_bitboard[sq];
}

Bitboard BitboardBase::square_to_bitboard_transpose(Square sq) const
{
	return _square_to_bitboard_transpose[sq];
}

Bitboard BitboardBase::square_to_bitboard_a1h8(Square sq) const
{
	return _square_to_bitboard_a1h8[sq];
}

Bitboard BitboardBase::square_to_bitboard_a8h1(Square sq) const
{
	return _square_to_bitboard_a8h1[sq];
}
  

void BitboardBase::init_square_to_bitboard()
{
	Bitboard tmp = 1;
	for (int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard[sq] = (tmp << sq);
	}
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

void BitboardBase::init_square_to_bitboard_transpose()
{
	Bitboard tmp = 1;
	for (int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_transpose[sq] = (tmp << (63 - sq / 8 - (sq % 8) * 8));
	}
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



void BitboardBase::init_square_to_bitboard_a1h8()
{
	Bitboard tmp = 1;
	int shift = 0;
	for (int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		shift = sq - (sq % 8) * 8;
		if (shift < 0) {  
			_square_to_bitboard_a1h8[sq] = (tmp << (64 + shift));
		}
		else {
			_square_to_bitboard_a1h8[sq] = (tmp << shift);
		}
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

void BitboardBase::init_square_to_bitboard_a8h1()
{
	Bitboard tmp = 1;
	int shift = 0;
	for (int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		shift = sq + (sq % 8) * 8;
		if (shift > 64) {  
			_square_to_bitboard_a8h1[sq] = (tmp << (shift - 64));
		}
		else {
			_square_to_bitboard_a8h1[sq] = (tmp << shift);
		}
	}
}

void BitboardBase::init_move_pos_board_rank()
{
	Bitrank rank_tmp;
	Bitboard board_tmp;
	for (int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		for (int rank_occup = 0; rank_occup < 256; ++rank_occup) {
			rank_tmp = rank_occup; 
			if (rank_tmp & (1 << sq % 8)) {
				rank_tmp = move_pos_rank(sq % 8, rank_tmp);
				board_tmp = (rank_tmp << (sq / 8) * 8);
				_move_pos_board_rank[sq][rank_occup] = board_tmp;
			}
			else {
				_move_pos_board_rank[sq][rank_occup] = 0;
			}
		}
	}
}

Bitrank BitboardBase::move_pos_rank(unsigned int position, Bitrank rank_occup) const
{
	return 0;
}
}
