#ifndef BITBOARDBASE_H_
#define BITBOARDBASE_H_

#include "utils.h"
namespace pismo
{

typedef uint8_t Bitrank;

class BitboardBase
{
public: 
	BitboardBase();
	
	Bitboard square_to_bitboard(Square sq) const;
	Bitboard square_to_bitboard_transpose(Square sq) const;
	Bitboard square_to_bitboard_a1h8(Square sq) const;
	Bitboard square_to_bitboard_a8h1(Square sq) const;

//private member functions
private:
	void init_square_to_bitboard();
	void init_square_to_bitboard_transpose();
	void init_square_to_bitboard_a1h8();
	void init_square_to_bitboard_a8h1();

	void init_move_pos_board_rank();
	Bitrank move_pos_rank(unsigned int position, Bitrank rank_occup) const;

// date members
private:

	Bitboard _square_to_bitboard[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_transpose[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_a1h8[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_a8h1[NUMBER_OF_SQUARES];
	
	Bitboard _move_pos_board_rank[64][256];
	Bitboard _move_pos_board_file[64][256];
	Bitboard _move_pos_board_diag_a1h8[64][256];
	Bitboard _move_pos_board_diag_a8h1[64][256];


};
}

#endif
