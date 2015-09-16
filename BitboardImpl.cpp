#include "BitboardImpl.h"
#include <assert.h>

namespace pismo
{

BitboardImpl::BitboardImpl()
{
	init_square_to_bitboard();
	init_square_to_bitboard_transpose();
	init_square_to_bitboard_a1h8();
	init_square_to_bitboard_a8h1();

	init_move_pos_board_rank();
	init_move_pos_board_file();
	init_move_pos_board_a1h8();
	init_move_pos_board_a8h1();

	init_move_pos_board_knight();
	init_move_pos_board_king();
	init_attacking_pos_board_pawn_white();
	init_attacking_pos_board_pawn_black();

	init_rank_pin_info();
	init_file_pin_info();
	init_diag_a1h8_pin_info();
	init_diag_a8h1_pin_info();
}

// Returns the bitboard with sq Square filled 
Bitboard BitboardImpl::square_to_bitboard(Square sq) const
{
	return _square_to_bitboard[sq];
}

// Returns the transposed bitboard with sq Square filled
Bitboard BitboardImpl::square_to_bitboard_transpose(Square sq) const
{
	return _square_to_bitboard_transpose[sq];
}

// Returns the 45 angle rotated bitboard with sq Square filled
Bitboard BitboardImpl::square_to_bitboard_diag_a1h8(Square sq) const
{
	return _square_to_bitboard_a1h8[sq];
}

// Returns the -45 angle rotated bitboard with sq Square filled
Bitboard BitboardImpl::square_to_bitboard_diag_a8h1(Square sq) const
{
	return _square_to_bitboard_a8h1[sq];
}

// Returns the bitboard for possible rank moves from Square from. 
// The occupied_squares bitboard is the unrotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::get_legal_rank_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank rank_occup = occupied_squares >> (from / 8) * 8;
	return _move_pos_board_rank[from][rank_occup];
}  

// Returns the transposed bitboard for possible file moves from Square from. 
// The occupied_squares bitboard is the transposed bitboard with occupied squares bit set to one
Bitboard BitboardImpl::get_legal_file_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank file_occup = occupied_squares >> (square_to_square_transpose(from) / 8) * 8;
	return _move_pos_board_file[from][file_occup];
}

// Returns the 45 angle rotated bitboard for possible a1h8 diagonal moves from Square from. 
// The occupied_squares bitboard is the 45 angle rotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::get_legal_diag_a1h8_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (square_to_square_a1h8(from) / 8) * 8;
	return _move_pos_board_diag_a1h8[from][diag_occup];
}

// Returns the -45 angle rotated bitboard for possible a8h1 diagonal moves from Square from. 
// The occupied_squares bitboard is the -45 angle rotated bitboard with occupied squares bit set to one
Bitboard BitboardImpl::get_legal_diag_a8h1_moves(Square from, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (square_to_square_a8h1(from) / 8) * 8;
	return _move_pos_board_diag_a8h1[from][diag_occup];
}

// Returns bitboard of possible knight moves from square from
Bitboard BitboardImpl::get_legal_knight_moves(Square from) const 
{
	return _move_pos_board_knight[from];
}

// Returns bitboard of possible king moves from square from
Bitboard BitboardImpl::get_legal_king_moves(Square from) const
{
	return _move_pos_board_king[from];
}

// Returns bitboard of possible white pawn attacking moves from square from
Bitboard BitboardImpl::get_legal_pawn_white_attacking_moves(Square from) const
{
	return _attacking_pos_board_pawn_white[from - A2];
}

// Returns bitboard of possible black pawn attacking moves from square from
Bitboard BitboardImpl::get_legal_pawn_black_attacking_moves(Square from) const
{
	return _attacking_pos_board_pawn_black[from - A2];
}

// Returns the bitboard of possible white pawn positions from where it can 
// check the king at position king_pos
Bitboard BitboardImpl::get_pawn_white_checking_pos(Square king_pos) const
{
	if (king_pos >= A4) {
		return _attacking_pos_board_pawn_white[king_pos - A4];
	}
	else if (king_pos >= A3) {
		return _attacking_pos_board_pawn_black[king_pos - A2];
	}
	else {
		return 0;
	}
}

// Returns the bitboard of possible black pawn positions from where it can
// check the king at position king_pos
Bitboard BitboardImpl::get_pawn_black_checking_pos(Square king_pos) const
{
	if (king_pos <= H5) {
		return _attacking_pos_board_pawn_black[king_pos + A2];
	}
	else if (king_pos <= H6) {
		return _attacking_pos_board_pawn_white[king_pos - A2];
	}
	else {
		return 0;
	}
}

// Returns pin_info for the rank ray when the king is at position king_sq
// occupied_squares is the normal bitboard of occupied squares 
const pin_info& BitboardImpl::get_rank_pin_info(Square king_sq, const Bitboard& occupied_squares) const
{
	Bitrank rank_occup = occupied_squares >> (king_sq / 8) * 8;
	return _rank_pin_info[king_sq][rank_occup];
}

// Returns pin_info for the file ray when the king is at position king_sq
// occupied_squares is the transposed bitboard of normal occupied squares 
const pin_info& BitboardImpl::get_file_pin_info(Square king_sq, const Bitboard& occupied_squares) const
{
	Bitrank file_occup = occupied_squares >> (square_to_square_transpose(king_sq) / 8) * 8;
	return _file_pin_info[king_sq][file_occup];
}

// Returns pin_info for a1h8 diagonal ray when the king is at position king_sq 
// occupied_squares is the 45 angle rotated bitboard of normal occupied squares
const pin_info& BitboardImpl::get_diag_a1h8_pin_info(Square king_sq, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (square_to_square_a1h8(king_sq) / 8) * 8;
	return _diag_a1h8_pin_info[king_sq][diag_occup];
}

// Returns pin_info for a8h1 diagonal ray when the king is at position king_sq 
// occupied_squares is the -45 angle rotated bitboard of normal occupied squares
const pin_info& BitboardImpl::get_diag_a8h1_pin_info(Square king_sq, const Bitboard& occupied_squares) const
{
	Bitrank diag_occup = occupied_squares >> (square_to_square_a8h1(king_sq) / 8) * 8;
	return _diag_a8h1_pin_info[king_sq][diag_occup];
}

// Changes the values of left_pos and right_pos to the squares where sliding piece and the king can be located
// when en_passant capture occurs which opens check possibility
// for example, if en_passant capture is from D5 to E6 then left_pos is the closest occupied square left to D5
// right_pos is the closest occupied square right to E5. If there is no piece they are set to INVALID_SQUARE
void BitboardImpl::get_en_passant_pin_info(Square from, Square to, const Bitboard& occupied_squares, Square& left_pos, Square& right_pos) const
{
	Bitrank rank_occup =  occupied_squares >> (from / 8) * 8;
	assert(rank_occup & (1 << (to % 8)));
	
	unsigned int left_pin;
	unsigned int right_pin;
	if ((to % 8) > (from % 8)) {
		left_pin = to % 8;
		right_pin = from % 8;
	}
	else {
		left_pin = from % 8;
		right_pin = to % 8;
	}
	int rsb_pos = find_msb_set(rank_occup << (8 - right_pin));
	int lsb_pos = find_lsb_set(rank_occup >> (left_pin + 1));
	left_pos = (rsb_pos != -1) ? (Square) ((from / 8) * 8 + (right_pin + rsb_pos - 8)) : INVALID_SQUARE;
	right_pos = (lsb_pos != -1) ? (Square) ((from / 8) * 8 + (left_pin + lsb_pos + 1)) : INVALID_SQUARE;
}


// Returns the converted normal bitboard from transpose bitboard
Bitboard BitboardImpl::bitboard_transpose_to_bitboard(const Bitboard& board_transpose) const
{
	Bitboard tmp;
	Bitboard result = board_transpose;
	const Bitboard mask1 = 0xF0F0F0F00F0F0F0F;
	const Bitboard mask2 = 0xCCCC0000CCCC0000;
	const Bitboard mask3 = 0xAA00AA00AA00AA00;
	tmp = result ^ (result << 36);
	result ^= mask1 & (tmp ^ (result >> 36));
	tmp = mask2 & (result ^ (result << 18));
	result ^= tmp ^ (tmp >> 18);
	tmp = mask3 & (result ^ (result << 9));
	result ^= tmp ^ (tmp >> 9);
	return result;
}

// Returns the converted normal bitboard from diagonal a1h8 bitboard
Bitboard BitboardImpl::bitboard_diag_a1h8_to_bitboard(const Bitboard& board_diag_a1h8) const
{
	Bitboard result = board_diag_a1h8;
	const Bitboard mask1 = 0x5555555555555555;
	const Bitboard mask2 = 0x3333333333333333;
	const Bitboard mask3 = 0x0F0F0F0F0F0F0F0F;
	result ^= mask1 & (result ^ rotate_bitboard_right(result, 8));
	result ^= mask2 & (result ^ rotate_bitboard_right(result, 16));
	result ^= mask3 & (result ^ rotate_bitboard_right(result, 32));
	return rotate_bitboard_right(result, 8);
}

// Returns the converted normal bitboard from diagonal a8h1 bitboard
Bitboard BitboardImpl::bitboard_diag_a8h1_to_bitboard(const Bitboard& board_diag_a8h1) const
{
	Bitboard result = board_diag_a8h1;
	const Bitboard mask1 = 0xAAAAAAAAAAAAAAAA;
	const Bitboard mask2 = 0xCCCCCCCCCCCCCCCC;
	const Bitboard mask3 = 0xF0F0F0F0F0F0F0F0;
	result ^= mask1 & (result ^ rotate_bitboard_right(result, 8));
	result ^= mask2 & (result ^ rotate_bitboard_right(result, 16));
	result ^= mask3 & (result ^ rotate_bitboard_right(result, 32));
	return result;
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
// Square to Square conversion from unrotated board to transposed board
Square BitboardImpl::square_to_square_transpose(Square sq) const
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
// Square to Square conversion from unrotated board to 45 angle rotated board
Square BitboardImpl::square_to_square_a1h8(Square sq) const
{
	int sq_a1h8 = sq - (sq % 8) * 8;
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
//Square to Square conversion from unrotated board to -45 angle rotated board
Square BitboardImpl::square_to_square_a8h1(Square sq) const
{
	int sq_a8h1 = sq + (sq % 8) * 8;
	if (sq_a8h1 > 64) {
		return (Square) (sq_a8h1 - 64);
	}
	else {
		return (Square) sq_a8h1;
	}
}

// Initializes square_to_bitboard array for all squares
void BitboardImpl::init_square_to_bitboard()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard[sq] = (tmp << sq);
	}
}

// Initializes square to transposed bitboard array for all squares 
void BitboardImpl::init_square_to_bitboard_transpose()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_transpose[sq] = (tmp << square_to_square_transpose((Square) sq));
	}
}

// Initializes square to 45 degree rotated bitboard array for all squares
void BitboardImpl::init_square_to_bitboard_a1h8()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_a1h8[sq] = (tmp << square_to_square_a1h8((Square) sq));
	}
}

// Initializes square to -45 degree rotated bitboard array for all squares
void BitboardImpl::init_square_to_bitboard_a8h1()
{
	Bitboard tmp = 1;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		_square_to_bitboard_a8h1[sq] = (tmp << square_to_square_a8h1((Square) sq));
	}
}

// Initializes rank movement position bitboard array for all squares and for all available
// occupations of the rank
void BitboardImpl::init_move_pos_board_rank()
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

// Initializes file movement position tranposed bitboard array for all squares and for 
// all available occupations of the file
void BitboardImpl::init_move_pos_board_file()
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

// Initializes diagonal a1h8 movement position 45 degree rotated bitboard array
// for all squares and for all available occupations of the diagonal
void BitboardImpl::init_move_pos_board_a1h8()
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

// Initializes diagonal a8h1 movement position -45 degree rotated bitboard array
// for all squares and for all available occupations of the diagonal
void BitboardImpl::init_move_pos_board_a8h1()
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

// Initializes knight movement position bitboard array for all squares
void BitboardImpl::init_move_pos_board_knight()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		switch (sq % 8) {
			case 0:
				_move_pos_board_knight[sq] = (sq <= A3 ? (KNIGHT_MOVES_A3 >> (A3 - sq)) : (KNIGHT_MOVES_A3 << (sq - A3)));
				break;
			case 1:
				_move_pos_board_knight[sq] = (sq <= B3 ? (KNIGHT_MOVES_B3 >> (B3 - sq)) : (KNIGHT_MOVES_B3 << (sq - B3)));
				break;
			case 6:
				_move_pos_board_knight[sq] = (sq <= G3 ? (KNIGHT_MOVES_G3 >> (G3 - sq)) : (KNIGHT_MOVES_G3 << (sq - G3)));
				break;
			case 7:
				_move_pos_board_knight[sq] = (sq <= H3 ? (KNIGHT_MOVES_H3 >> (H3 - sq)) : (KNIGHT_MOVES_H3 << (sq - H3)));
				break;
			default:
				_move_pos_board_knight[sq] = (sq <= C3 ? (KNIGHT_MOVES_C3 >> (C3 - sq)) : (KNIGHT_MOVES_C3 << (sq - C3)));
		}
	}
}

// Initializes king movement position bitboard array for all squares
void BitboardImpl::init_move_pos_board_king()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		switch (sq % 8) {
			case 0:
				_move_pos_board_king[sq] = (sq <= A2 ? (KING_MOVES_A2 >> (A2 - sq)) : (KING_MOVES_A2 << (sq - A2)));
				break;
			case 7:
				_move_pos_board_king[sq] = (sq <= H2 ? (KING_MOVES_H2 >> (H2 - sq)) : (KING_MOVES_H2 << (sq - H2)));
				break;
			default:
				_move_pos_board_king[sq] = (sq <= B2 ? (KING_MOVES_B2 >> (B2 - sq)) : (KING_MOVES_B2 << (sq - B2)));
		}
	}
}

// Initializes white pawn attacking position bitboard array for all squares
// where white pawn allowed to be
void BitboardImpl::init_attacking_pos_board_pawn_white()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		if (sq % 8 == 0) {
			_attacking_pos_board_pawn_white[sq - A2] = (PAWN_WHITE_ATTACK_A2 << (sq - A2));
		}
		else if (sq % 8 == 7) {
			_attacking_pos_board_pawn_white[sq - A2] = (PAWN_WHITE_ATTACK_H2 << (sq - H2));
		}
		else {
			_attacking_pos_board_pawn_white[sq - A2] = (PAWN_WHITE_ATTACK_B2 << (sq - B2));
		}
	}
}

// Initializes black pawn attacking position bitboard array for all squares
// where black pawn allowed to be
void BitboardImpl::init_attacking_pos_board_pawn_black()
{
	for (unsigned int sq = A2; sq <= H7; ++sq) {
		if (sq % 8 == 0) {
			_attacking_pos_board_pawn_black[sq - A2] = (PAWN_BLACK_ATTACK_A7 >> (A7 - sq));
		}
		else if (sq % 8 == 7) {
			_attacking_pos_board_pawn_black[sq - A2] = (PAWN_BLACK_ATTACK_H7 >> (H7 - sq));
		}
		else {
			_attacking_pos_board_pawn_black[sq - A2] = (PAWN_BLACK_ATTACK_G7 >> (G7 - sq));
		}
	}
}

// Initializes _rank_pin_info array for all possible rank occupations and 
// for all squares where king can be located
// pin_info is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the left_slide square is smaller than king square
// and right_slide is bigger than king square
// in left_board and right_board Bitboards positions of pinned pieces are stored plus
// the positions of sliding pieces 
void BitboardImpl::init_rank_pin_info()
{
	Bitrank left_rank;
	Bitrank right_rank;
	Bitboard left_board;
	Bitboard right_board;
	int left_slide_pos;
	int right_slide_pos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		for (unsigned int rank_occup = 0; rank_occup < 256; ++rank_occup) {
			if (rank_occup & (1 << sq % 8)) {
				set_rank_pin_info(sq % 8, rank_occup, right_slide_pos, left_slide_pos, right_rank, left_rank);
				left_board = left_rank;
				right_board = right_rank;
				if (left_board) {
					left_board <<= (sq / 8) * 8;
				}
				if (right_board) {
					right_board <<= (sq / 8) * 8;
				}
				Square left_slide = (left_slide_pos != -1) ? (Square) ((sq / 8) * 8 + left_slide_pos) : INVALID_SQUARE;
				Square right_slide = (right_slide_pos != -1) ? (Square) ((sq / 8) * 8 + right_slide_pos) : INVALID_SQUARE;
				_rank_pin_info[sq][rank_occup] = pin_info(left_slide, right_slide, left_board, right_board);
			}
			else {
				_rank_pin_info[sq][rank_occup] = pin_info();
			}
		}
	}
}

// Initializes _file_pin_info array for all possible file occupations and 
// for all squares where king can be located
// pin_info is comprised from sliding piece positions to the up and down of king position 
// in this case the square of the up_slide square is bigger than king square
// and down_slide is smaller than king square
// in up_board and down_board Bitboards positions of pinned pieces are stored plus the
// the positions of sliding pieces; Bitboards in the pin_info are transposed of normal Bitboards
void BitboardImpl::init_file_pin_info()
{
	Bitrank up_file;
	Bitrank down_file;
	Bitboard up_board;
	Bitboard down_board;
	int up_slide_pos;
	int down_slide_pos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_transpose = square_to_square_transpose((Square) sq);
		for (unsigned int file_occup = 0; file_occup < 256; ++file_occup) {
			if (file_occup & (1 << sq_transpose % 8)) {
				set_rank_pin_info(sq_transpose % 8, file_occup, down_slide_pos, up_slide_pos, down_file, up_file);
				up_board = up_file;
				down_board = down_file;
				if (up_board) {
					up_board  <<= (sq_transpose / 8) * 8;
				}
				if (down_board) {
					down_board <<= (sq_transpose / 8) * 8;
				}
				Square up_slide = (up_slide_pos != -1) ? square_to_square_transpose((Square) ((sq_transpose / 8) * 8 + up_slide_pos)) : INVALID_SQUARE;
				Square down_slide = (down_slide_pos != -1) ? square_to_square_transpose((Square) ((sq_transpose / 8) * 8 + down_slide_pos)) : INVALID_SQUARE;
				_file_pin_info[sq][file_occup] = pin_info(down_slide, up_slide, down_board, up_board);
			}
			else {
				_file_pin_info[sq][file_occup] = pin_info();
			}
		}
	}
}

// Initializes _diag_a1h8_pin_info array for all possible a1h8 diagonal occupations and 
// for all squares where king can be located
// pin_info is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the right_slide square is bigger than king square
// and left_slide is smaller than king square
// in right_board and left_board Bitboards positions of the pinned pieces is stored plus
// the positions of sliding pieces; Bitboards in pin_info are 45 degree rotated form of the normal Bitboards
void BitboardImpl::init_diag_a1h8_pin_info()
{
	Bitrank left_diag;
	Bitrank right_diag;
	Bitrank diag_allowed;
	Bitboard left_board;
	Bitboard right_board;
	int left_slide_pos;
	int right_slide_pos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_a1h8 = square_to_square_a1h8((Square) sq);
		if (sq_a1h8 % 8 < 8 - sq_a1h8 / 8) {
			diag_allowed = OCCUPATION_FROM_LSB[8 - sq_a1h8 / 8];
		}
		else {
			diag_allowed = ~OCCUPATION_FROM_LSB[8 - sq_a1h8 / 8];
		}
		for (unsigned int diag_occup = 0; diag_occup < 256; ++diag_occup) {
			if (diag_occup & (1 << sq_a1h8 % 8)) {
				set_rank_pin_info(sq_a1h8 % 8, diag_occup & diag_allowed, right_slide_pos, left_slide_pos, right_diag, left_diag);
				left_board = left_diag;
				right_board = right_diag;
				if (left_board) {
					left_board  <<= (sq_a1h8 / 8) * 8;
				}
				if (right_board) {
					right_board <<= (sq_a1h8 / 8) * 8;
				}
				Square left_slide = (left_slide_pos != -1) ? square_to_square_a8h1((Square) ((sq_a1h8 / 8) * 8 + left_slide_pos)) : INVALID_SQUARE;
				Square right_slide = (right_slide_pos != -1) ? square_to_square_a8h1((Square) ((sq_a1h8 / 8) * 8 + right_slide_pos)) : INVALID_SQUARE;
				_diag_a1h8_pin_info[sq][diag_occup] = pin_info(left_slide, right_slide, left_board, right_board);
			}
			else {
				_diag_a1h8_pin_info[sq][diag_occup] = pin_info();
			}
		}
	}
}

// Initializes _diag_a8h1_pin_info array for all possible a8h1 diagonal occupations and 
// for all squares where king can be located
// pin_info is comprised from sliding piece positions to the left and right of king position 
// in this case the square of the right_slide square is smaller than king square
// and left_slide is bigger than king square
// in right_board and left_board Bitboards positions of the pinned pieces is stored plus
// the positions of the sliding pieces; Bitboards in pin_info are -45 degree rotated form of the normal Bitboards
void BitboardImpl::init_diag_a8h1_pin_info()
{
	Bitrank left_diag;
	Bitrank right_diag;
	Bitrank diag_allowed;
	Bitboard left_board;
	Bitboard right_board;
	int left_slide_pos;
	int right_slide_pos;
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		Square sq_a8h1 = square_to_square_a8h1((Square) sq);
		if (sq_a8h1 % 8 < sq_a8h1 / 8 + 1) {
			diag_allowed = OCCUPATION_FROM_LSB[sq_a8h1 / 8 + 1];
		}
		else {
			diag_allowed = ~OCCUPATION_FROM_LSB[sq_a8h1 / 8 + 1];
		}
		for (unsigned int diag_occup = 0; diag_occup < 256; ++diag_occup) {
			if (diag_occup & (1 << sq_a8h1 % 8)) {
				set_rank_pin_info(sq_a8h1 % 8, diag_occup & diag_allowed, right_slide_pos, left_slide_pos, right_diag, left_diag);
				left_board = left_diag;
				right_board = right_diag;
				if (left_board) {
					left_board <<= (sq_a8h1 / 8) * 8;
				}
				if (right_board) {
					right_board <<= (sq_a8h1 / 8) * 8;
				}
				Square left_slide = (left_slide_pos != -1) ? square_to_square_a1h8((Square) ((sq_a8h1 / 8) * 8 + left_slide_pos)) : INVALID_SQUARE;
				Square right_slide = (right_slide_pos != -1) ? square_to_square_a1h8((Square) ((sq_a8h1 / 8) * 8 + right_slide_pos)) : INVALID_SQUARE;
				_diag_a8h1_pin_info[sq][diag_occup] = pin_info(right_slide, left_slide, right_board, left_board);
			}
			else {
				_diag_a8h1_pin_info[sq][diag_occup] = pin_info();
			}
		}
	}
}

// Returns Bitrank of possible moves for rank_occup occupied bitrank when the piece is at position
// for example for the Bitrank 10101011 and the position 3 it returns 00110110
Bitrank BitboardImpl::move_pos_rank(unsigned int position, Bitrank rank_occup) const
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

// Sets left_pin to possible positions where pinned piece can be located plus the sliding piece position
// left to the attacked piece at position
// Sets right_pin to possible positions where pinned piece can be located plus the sliding piece position
// right to the attacked piece at position 
// left_slide_pos is the position of the sliding piece to the left; -1 if not there
// right_slide_pos is the position of the sliding piece to the right; -1 if not there
// for example for the Bitrank 10101011 and position 3, the left_pin is 11110000
// left_slide_pos is 7, right_pin is 00000111, right_slide_pos is 0
void BitboardImpl::set_rank_pin_info(unsigned int position, Bitrank rank_occup, int& left_slide_pos, int& right_slide_pos, Bitrank& left_pin, Bitrank& right_pin) const
{
	int right_set_bit = find_msb_set(rank_occup << (8 - position));
	int right_pin_pos = (right_set_bit != -1) ? (position - 8 + right_set_bit) : -1;
	int right_next_set_bit = (right_pin_pos != -1) ? find_msb_set(rank_occup << (8 - right_pin_pos)) : -1;
	right_slide_pos = (right_next_set_bit != -1) ? (right_pin_pos - 8 + right_next_set_bit) : -1;
	int left_set_bit = find_lsb_set(rank_occup >> (1 + position));
	int left_pin_pos = (left_set_bit != -1) ? (position + 1 + left_set_bit) : -1;
	int left_next_set_bit = (left_pin_pos != -1) ? find_lsb_set(rank_occup >> (1 + left_pin_pos)) : -1;
	left_slide_pos = (left_next_set_bit != -1) ? (left_pin_pos + 1 + left_next_set_bit) : -1;
	left_pin = (left_slide_pos != -1) ? OCCUPATION_FROM_LSB[left_slide_pos + 1] ^ OCCUPATION_FROM_LSB[position + 1] : 0;
	right_pin = (right_slide_pos != -1) ? OCCUPATION_FROM_LSB[position] ^ OCCUPATION_FROM_LSB[right_slide_pos] : 0;
}

// Returns the position of least significant bit set in the Bitrank
// Uses binary search algorithm
int BitboardImpl::find_lsb_set(Bitrank rank) const 
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

// Returns the position of the most significant bit set in the Bitrank
// Uses binary search algorithm
int BitboardImpl::find_msb_set(Bitrank rank) const
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

// rotates the bits of bitboard to the right
// Here rotate is similar to the shift operation, but wraps
// over the edge bits around
Bitboard BitboardImpl::rotate_bitboard_right(const Bitboard& board, unsigned int num) const
{
	return ((board >> num) | (board << (64 - num)));
}
}
