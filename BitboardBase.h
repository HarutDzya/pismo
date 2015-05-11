#ifndef BITBOARDBASE_H_
#define BITBOARDBASE_H_

#include "utils.h"
namespace pismo
{

typedef uint8_t Bitrank;

const Bitrank OCCUPATION_FROM_LSB[] =
	{0x00, 0x01, 0x03, 0x07, 0x0F, 
	0x1F, 0x3F, 0x7F, 0xFF}; 

class BitboardBase
{
public: 
	BitboardBase();
	
	Bitboard square_to_bitboard(Square sq) const;
	Bitboard square_to_bitboard_transpose(Square sq) const;
	Bitboard square_to_bitboard_a1h8(Square sq) const;
	Bitboard square_to_bitboard_a8h1(Square sq) const;
	
	Bitboard get_legal_rank_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_file_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_diag_a1h8_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_diag_a8h1_moves(Square from, const Bitboard& occupied_squares) const;
	
//private member functions
private:
	Square square_to_square_transpose(Square sq) const;
	Square square_to_square_a1h8(Square sq) const;
	Square square_to_square_a8h1(Square sq) const;

	void init_square_to_bitboard();
	void init_square_to_bitboard_transpose();
	void init_square_to_bitboard_a1h8();
	void init_square_to_bitboard_a8h1();

	void init_move_pos_board_rank();
	void init_move_pos_board_file();
	void init_move_pos_board_a1h8();
	void init_move_pos_board_a8h1();
	
	Bitrank move_pos_rank(unsigned int position, Bitrank rank_occup) const;
	int find_lsb_set(Bitrank rank) const;
	int find_msb_set(Bitrank rank) const;

// date members
private:

	Bitboard _square_to_bitboard[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_transpose[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_a1h8[NUMBER_OF_SQUARES];
	Bitboard _square_to_bitboard_a8h1[NUMBER_OF_SQUARES];

	// TODO: For optimization purposes change arrays to [64][64]	
	Bitboard _move_pos_board_rank[NUMBER_OF_SQUARES][256];
	Bitboard _move_pos_board_file[NUMBER_OF_SQUARES][256];
	Bitboard _move_pos_board_diag_a1h8[NUMBER_OF_SQUARES][256];
	Bitboard _move_pos_board_diag_a8h1[NUMBER_OF_SQUARES][256];


};
}

#endif
