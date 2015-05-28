#ifndef BITBOARDIMPL_H_
#define BITBOARDIMPL_H_

#include "utils.h"
#include <stdint.h>

namespace pismo
{

typedef uint64_t Bitboard;
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
const Bitboard KING_MOVES_H2 = 0x0000000000C080C0; // Bitboard for possible moves of king at Square H2 

const Bitrank OCCUPATION_FROM_LSB[] =
	{0x00, 0x01, 0x03, 0x07, 0x0F, 
	0x1F, 0x3F, 0x7F, 0xFF}; 

class BitboardImpl
{
public: 
	BitboardImpl();
	
	Bitboard square_to_bitboard(Square sq) const;
	Bitboard square_to_bitboard_transpose(Square sq) const;
	Bitboard square_to_bitboard_diag_a1h8(Square sq) const;
	Bitboard square_to_bitboard_diag_a8h1(Square sq) const;
	
	Bitboard get_legal_rank_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_file_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_diag_a1h8_moves(Square from, const Bitboard& occupied_squares) const;
	Bitboard get_legal_diag_a8h1_moves(Square from, const Bitboard& occupied_squares) const;

	Bitboard get_legal_knight_moves(Square from) const;
	Bitboard get_legal_king_moves(Square from) const;
	Bitboard get_legal_pawn_white_attacking_moves(Square from) const;
	Bitboard get_legal_pawn_black_attacking_moves(Square from) const;

	Bitboard bitboard_transpose_to_bitboard(const Bitboard& board_transpose) const;
	Bitboard bitboard_diag_a1h8_to_bitboard(const Bitboard& board_diag_a1h8) const;
	Bitboard bitboard_diag_a8h1_to_bitboard(const Bitboard& board_diag_a8h1) const;

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

	void init_move_pos_board_knight();
	void init_move_pos_board_king();
	void init_attacking_pos_board_pawn_white();
	void init_attacking_pos_board_pawn_black();
	
	Bitrank move_pos_rank(unsigned int position, Bitrank rank_occup) const;
	int find_lsb_set(Bitrank rank) const;
	int find_msb_set(Bitrank rank) const;
	
	Bitboard rotate_bitboard_right(const Bitboard& board, unsigned int num) const;

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

	Bitboard _move_pos_board_knight[NUMBER_OF_SQUARES];
	Bitboard _move_pos_board_king[NUMBER_OF_SQUARES];
	Bitboard _attacking_pos_board_pawn_white[NUMBER_OF_SQUARES];
	Bitboard _attacking_pos_board_pawn_black[NUMBER_OF_SQUARES];


};
}

#endif
