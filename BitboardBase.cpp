#include "BitboardBase.h"

namespace pismo
{

BitboardBase::BitboardBase()
{
	init_square_to_bitboard();
	init_square_to_bitboard_transpose();
	init_square_to_bitboard_a1h8();
	init_square_to_bitboard_a8h1();

	init_move_pos_board_rank();
	init_move_pos_board_file();
	init_move_pos_board_a1h8();
	init_move_pos_board_a8h1();
}


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

Bitboard BitboardBase::get_legal_rank_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank rank_occup = occupied_squares >> (from / 8) * 8;
	return _move_pos_board_rank[from][rank_occup];
}  

Bitboard BitboardBase::get_legal_file_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank file_occup = occupied_squares >> (from / 8) * 8;
	return _move_pos_board_file[from][file_occup];
}

Bitboard BitboardBase::get_legal_diag_a1h8_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (from / 8) * 8;
	return _move_pos_board_diag_a1h8[from][diag_occup];
}

Bitboard BitboardBase::get_legal_diag_a8h1_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (from / 8) * 8;
	return _move_pos_board_diag_a8h1[from][diag_occup];
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

Square BitboardBase::square_to_square_transpose(Square sq) const
{
	return (Square) (63 - sq / 8 - (sq % 8) * 8);
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

Square BitboardBase::square_to_square_a1h8(Square sq) const
{
	unsigned int sq_a1h8 = sq - (sq % 8) * 8;
	if (sq_a1h8 < 0) {
		return (Square) (64 + sq_a1h8);
	}
	else {
		return (Square) sq_a1h8;
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

Square BitboardBase::square_to_square_a8h1(Square sq) const
{
	unsigned int sq_a8h1 = sq + (sq % 8) * 8;
	if (sq_a8h1 > 64) {
		return (Square) (sq_a8h1 - 64);
	}
	else {
		return (Square) sq_a8h1;
	}
}

void BitboardBase::init_square_to_bitboard()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard[sq] = (tmp << sq);
	}
}

void BitboardBase::init_square_to_bitboard_transpose()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_transpose[sq] = (tmp << square_to_square_transpose((Square) sq));
	}
}

void BitboardBase::init_square_to_bitboard_a1h8()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_a1h8[sq] = (tmp << square_to_square_a1h8((Square) sq));
	}
}

void BitboardBase::init_square_to_bitboard_a8h1()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_a8h1[sq] = (tmp << square_to_square_a8h1((Square) sq));
	}
}

void BitboardBase::init_move_pos_board_rank()
{
	Bitrank rank_tmp;
	Bitboard board_tmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		for (unsigned int rank_occup = 0; rank_occup < 256; ++rank_occup) {
			rank_tmp = rank_occup; 
			if (rank_tmp & (1 << sq % 8)) {
				rank_tmp = move_pos_rank(sq % 8, rank_tmp);
				board_tmp = rank_tmp;
				board_tmp  <<= (sq / 8) * 8;
				_move_pos_board_rank[sq][rank_occup] = board_tmp;
			}
			else {
				_move_pos_board_rank[sq][rank_occup] = 0;
			}
		}
	}
}

void BitboardBase::init_move_pos_board_file()
{
	Bitrank file_tmp;
	Bitboard board_tmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_transpose = square_to_square_transpose((Square) sq);
		for (unsigned int file_occup = 0; file_occup < 256; ++file_occup) {
			file_tmp = file_occup; 
			if (file_tmp & (1 << sq_transpose % 8)) {
				file_tmp = move_pos_rank(sq_transpose % 8, file_tmp);
				board_tmp = file_tmp;
				board_tmp  <<= (sq_transpose / 8) * 8;
				_move_pos_board_file[sq][file_occup] = board_tmp;
			}
			else {
				_move_pos_board_file[sq][file_occup] = 0;
			}
		}
	}
}

void BitboardBase::init_move_pos_board_a1h8()
{
	Bitrank diag_a1h8_tmp;
	Bitrank diag_allowed;
	Bitboard board_tmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_a1h8 = square_to_square_a1h8((Square) sq);
		if (sq_a1h8 % 8 < 8 - sq_a1h8 / 8) {
			diag_allowed = OCCUPATION_FROM_LSB[8 - sq_a1h8 / 8];
		}
		else {
			diag_allowed = ~OCCUPATION_FROM_LSB[8 - sq_a1h8 / 8];
		}
		for (unsigned int diag_occup = 0; diag_occup < 256; ++diag_occup) {
			diag_a1h8_tmp = diag_occup; 
			if (diag_a1h8_tmp & (1 << sq_a1h8 % 8)) {
				diag_a1h8_tmp = move_pos_rank(sq_a1h8 % 8, diag_a1h8_tmp);
				board_tmp = (diag_a1h8_tmp & diag_allowed);
				board_tmp  <<= (sq_a1h8 / 8) * 8;
				_move_pos_board_diag_a1h8[sq][diag_occup] = board_tmp;
			}
			else {
				_move_pos_board_diag_a1h8[sq][diag_occup] = 0;
			}
		}
	}
}

void BitboardBase::init_move_pos_board_a8h1()
{
	Bitrank diag_a8h1_tmp;
	Bitrank diag_allowed;
	Bitboard board_tmp;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_a8h1 = square_to_square_a8h1((Square) sq);
		if (sq_a8h1 % 8 < sq_a8h1 / 8 + 1) {
			diag_allowed = OCCUPATION_FROM_LSB[sq_a8h1 / 8 + 1];
		}
		else {
			diag_allowed = ~OCCUPATION_FROM_LSB[sq_a8h1 / 8 + 1];
		}
		for (unsigned int diag_occup = 0; diag_occup < 256; ++diag_occup) {
			diag_a8h1_tmp = diag_occup; 
			if (diag_a8h1_tmp & (1 << sq_a8h1 % 8)) {
				diag_a8h1_tmp = move_pos_rank(sq_a8h1 % 8, diag_a8h1_tmp);
				board_tmp = (diag_a8h1_tmp & diag_allowed);
				board_tmp  <<= (sq_a8h1 / 8) * 8;
				_move_pos_board_diag_a8h1[sq][diag_occup] = board_tmp;
			}
			else {
				_move_pos_board_diag_a8h1[sq][diag_occup] = 0;
			}
		}
	}
}

Bitrank BitboardBase::move_pos_rank(unsigned int position, Bitrank rank_occup) const
{
	int right_set_bit = find_msb_set(rank_occup << (8 - position));
	int left_set_bit = find_lsb_set(rank_occup >> (1 + position));
	if (right_set_bit == -1 && left_set_bit == -1) {
		return OCCUPATION_FROM_LSB[8] ^ (1 << position);
	}
	else if (right_set_bit == -1) {
		return OCCUPATION_FROM_LSB[position + 2 + left_set_bit] ^ (1 << position);
	}
	else if (left_set_bit == -1) {
		return OCCUPATION_FROM_LSB[8] ^ OCCUPATION_FROM_LSB[position - 8 + right_set_bit] ^ (1 << position);
	}
	else {
		return OCCUPATION_FROM_LSB[position - 8 + right_set_bit] ^ 
		OCCUPATION_FROM_LSB[position + 2 + left_set_bit] ^ (1 << position);	
	}
	return 0;
}

int BitboardBase::find_lsb_set(Bitrank rank) const 
{
	if (rank) {
		if (rank & 0x01) {
			return 0;
		}
		else {
			int result = 1;
			if ((rank & 0x0F) == 0) {
				rank >>= 4;
				result += 4;
			}
			if ((rank & 0x03) == 0) {
				rank >>= 2;
				result += 2;
			}
			return result - (rank & 0x01);
		}
	}

	return -1;
}

int BitboardBase::find_msb_set(Bitrank rank) const
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
}
