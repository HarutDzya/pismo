#include "PossibleMoves.h"

namespace pismo
{

PossibleMoves::PossibleMoves()
{
	init_white_pawn_moves();
	init_black_pawn_moves();
	init_knight_moves();
	init_king_moves();
	init_left_rank_moves();
	init_right_rank_moves();
	init_up_file_moves();
	init_down_file_moves();
	init_up_diag_a1h8_moves();
	init_down_diag_a1h8_moves();
	init_up_diag_a8h1_moves();
	init_down_diag_a8h1_moves();
}

const std::vector<Square>& PossibleMoves::possible_white_pawn_moves(Square from) const
{
	return _white_pawn_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_black_pawn_moves(Square from) const
{
	return _black_pawn_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_knight_moves(Square from) const
{
	return _knight_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_king_moves(Square from) const
{
	return _king_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_left_rank_moves(Square from) const
{
	return _left_rank_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_right_rank_moves(Square from) const
{
	return _right_rank_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_up_file_moves(Square from) const
{
	return _up_file_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_down_file_moves(Square from) const
{
	return _down_file_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_up_diag_a1h8_moves(Square from) const
{
	return _up_diag_a1h8_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_down_diag_a1h8_moves(Square from) const
{
	return _down_diag_a1h8_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_up_diag_a8h1_moves(Square from) const
{
	return _up_diag_a8h1_moves_list[from];
}

const std::vector<Square>& PossibleMoves::possible_down_diag_a8h1_moves(Square from) const
{
	return _down_diag_a8h1_moves_list[from];
}

void PossibleMoves::init_white_pawn_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		if (sq <= H1 || sq >= A8) {
			_white_pawn_moves_list.push_back(std::vector<Square>());
		}
		else {
			std::vector<Square> pawn_moves_square;
			pawn_moves_square.push_back((Square) (sq + 8));
			if ((sq % 8) > (A1 % 8)) { 
				pawn_moves_square.push_back((Square) (sq + 7));
			}
			if ((sq % 8) < (H1 % 8)) {
				pawn_moves_square.push_back((Square) (sq + 9));
			}
			if (sq >= A2 && sq <= H2) {
				pawn_moves_square.push_back((Square) (sq + 16));
			}
			_white_pawn_moves_list.push_back(pawn_moves_square);
		}
	}
}

void PossibleMoves::init_black_pawn_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		if (sq <= H1 || sq >= A8) {
			_black_pawn_moves_list.push_back(std::vector<Square>());
		}
		else {
			std::vector<Square> pawn_moves_square;
			pawn_moves_square.push_back((Square) (sq - 8));
			if ((sq % 8) < (H1 % 8)) {
				pawn_moves_square.push_back((Square) (sq - 7));
			}
			if ((sq % 8) > (A1 % 8)) { 
				pawn_moves_square.push_back((Square) (sq - 9));
			}
			if (sq >= A7 && sq <= H7) {
				pawn_moves_square.push_back((Square) (sq - 16));
			}
			_black_pawn_moves_list.push_back(pawn_moves_square);
		}
	}
}

void PossibleMoves::init_knight_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> knight_moves_square;
		if ((sq / 8 >= A3 / 8) && (sq % 8 >= B3 % 8)) {
			knight_moves_square.push_back((Square) (sq - 17));
		}
		if ((sq / 8 >= A2 / 8) && (sq % 8 >= C2 % 8)) {
			knight_moves_square.push_back((Square) (sq - 10));
		}
		if ((sq / 8 <= A7 / 8) && (sq % 8 >= C7 % 8)) {
			knight_moves_square.push_back((Square) (sq + 6));
		}
		if ((sq / 8 <= A6 / 8) && (sq % 8 >= B6 % 8)) {
			knight_moves_square.push_back((Square) (sq + 15));
		}
		if ((sq / 8 <= A6 / 8) && (sq % 8 <= G6 % 8)) {
			knight_moves_square.push_back((Square) (sq + 17));
		}
		if ((sq / 8 <= A7 / 8) && (sq % 8 <= F7 % 8)) {
			knight_moves_square.push_back((Square) (sq + 10));
		}
		if ((sq / 8 >= A2 / 8) && (sq % 8 <= F2 % 8)) {
			knight_moves_square.push_back((Square) (sq - 6));
		}
		if ((sq / 8 >= A3 / 8) && (sq % 8 <= G3 % 8)) {
			knight_moves_square.push_back((Square) (sq - 15));
		}		
		_knight_moves_list.push_back(knight_moves_square);
	}
}

void PossibleMoves::init_king_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> king_moves_square;
		if (sq / 8 >= A2 / 8) {
			if (sq % 8 <= G2 % 8) {
				king_moves_square.push_back((Square) (sq - 7));
			}
			king_moves_square.push_back((Square) (sq - 8));
			if (sq % 8 >= B2 % 8) {
				king_moves_square.push_back((Square) (sq - 9));
			}
		}
		if (sq % 8 >= B2 % 8) {
			king_moves_square.push_back((Square) (sq - 1));
		}
		if (sq / 8 <= A7 / 8) {
			if (sq % 8 >= B2 % 8) {
				king_moves_square.push_back((Square) (sq + 7));
			}
			king_moves_square.push_back((Square) (sq + 8));
			if (sq % 8 <= G2 % 8) {
				king_moves_square.push_back((Square) (sq + 9));
			}
		}
		if (sq % 8 <= G2 % 8) {
			king_moves_square.push_back((Square) (sq + 1));
		}
		if (sq == E1) {
			king_moves_square.push_back(C1);
			king_moves_square.push_back(G1);
		}
		if (sq == E8) {
			king_moves_square.push_back(C8);
			king_moves_square.push_back(G8);
		}
		_king_moves_list.push_back(king_moves_square);
	}
}

// The left rank moves are initialized starting from one left of the current square 
// position until the left end of the rank  
void PossibleMoves::init_left_rank_moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> left_rank_moves_square;
		for (int sq_to = sq - 1 ; sq_to >= A1 && (sq / 8 - sq_to / 8) == 0; --sq_to) {
		       left_rank_moves_square.push_back((Square) sq_to);
		}
		_left_rank_moves_list.push_back(left_rank_moves_square);
	}
}

// The right rank moves are initialized starting from one right of the current square 
// position until the right end of the rank  
void PossibleMoves::init_right_rank_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> right_rank_moves_square;
		for (unsigned int sq_to = sq + 1 ; sq_to <= H8 && (sq_to / 8 - sq / 8) == 0; ++sq_to) {
		       right_rank_moves_square.push_back((Square) sq_to);
		}
		_right_rank_moves_list.push_back(right_rank_moves_square);
	}
}

// The up file moves are initialized starting from one square up of the current
// square position until the end of the file
void PossibleMoves::init_up_file_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> up_file_moves_square;
		for (unsigned int sq_to = sq + 8; sq_to <= H8; sq_to += 8) {
			up_file_moves_square.push_back((Square) sq_to);
		}
		_up_file_moves_list.push_back(up_file_moves_square);
	}
}

// The down file moves are initialized starting from one square down of the current
// square position until the end of the file
void PossibleMoves::init_down_file_moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> down_file_moves_square;
		for (int sq_to = sq - 8; sq_to >= A1; sq_to -= 8) {
			down_file_moves_square.push_back((Square) sq_to);
		}
		_down_file_moves_list.push_back(down_file_moves_square);
	}
}

// The up diagonal A1H8 moves are initialized starting from one square up 
// on diagonal of the current square position until the end of diagonal  
void PossibleMoves::init_up_diag_a1h8_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> up_diag_a1h8_moves_square;
		for (unsigned int sq_to = sq + 9; sq_to <= H8 && (sq_to / 8 - sq / 8) == (sq_to - sq) / 9; sq_to += 9) {
		       up_diag_a1h8_moves_square.push_back((Square) sq_to);
		}
 		_up_diag_a1h8_moves_list.push_back(up_diag_a1h8_moves_square);
	}
}

// The down diagonal A1H8 moves are initialized starting from one square 
// down on diagonal of the current square position until the end of diagonal
void PossibleMoves::init_down_diag_a1h8_moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> down_diag_a1h8_moves_square;
		for (int sq_to = sq - 9; sq_to >= A1 && (sq / 8 - sq_to / 8) == (sq - sq_to) / 9; sq_to -= 9) {
		       down_diag_a1h8_moves_square.push_back((Square) sq_to);
		}
 		_down_diag_a1h8_moves_list.push_back(down_diag_a1h8_moves_square);
	}
}

// The up diagonal A8H1 moves are initialized starting from one square up
// on diagonal of the current square position until the end of diagonal
void PossibleMoves::init_up_diag_a8h1_moves()
{
	for (unsigned int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> up_diag_a8h1_moves_square;
		for (unsigned int sq_to = sq + 7; sq_to <= H8 && (sq_to / 8 - sq / 8) == (sq_to - sq) / 7; sq_to += 7) {
			up_diag_a8h1_moves_square.push_back((Square) sq_to);
		}
		_up_diag_a8h1_moves_list.push_back(up_diag_a8h1_moves_square);
	}
}

// The down diagonal A8H1 moves are initialized starting from one square
// down on diagonal of the current square position until the end of diagonal
void PossibleMoves::init_down_diag_a8h1_moves()
{
	for (int sq = A1; sq <= H8; ++sq) {
		std::vector<Square> down_diag_a8h1_moves_square;
		for (int sq_to = sq - 7; sq_to >= A1 && (sq / 8 - sq_to / 8) == (sq - sq_to) / 7; sq_to -= 7) {
			down_diag_a8h1_moves_square.push_back((Square) sq_to);
		}
		_down_diag_a8h1_moves_list.push_back(down_diag_a8h1_moves_square);
	}
}
}
