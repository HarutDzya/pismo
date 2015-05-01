#ifndef POSITIONSTATE_H_
#define POSITIONSTATE_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class PositionState
{
public:
	PositionState();


	void set_piece(Square s, Piece p);

	void init_position(const std::vector<std::pair<Square, Piece> >& pieces); 
	/*
	Checks all the rules of the game for legality of the move
	even if it involves capture of the opponent piece
	*/
	bool move_is_legal(const move_info& move) const;
	
	/*
	Makes a move if the move if legal according to the move_is_legal
	method
	*/
	//Temporary changed the return value to bool
	void make_move(const move_info& move);
	
	
	/*
	Prints board for white pieces using information from 
	_white_pieces Bitboard
	*/
	void print_white_pieces() const;
	
	/*
	Prints board for black pieces using information from 
	_black_pieses Bitboard
	*/
	void print_black_pieces() const;


	/*
	Prints single board for both pieces using information
	from _board array
	*/
	void print_board() const;

//private member functions
private:
	bool pawn_move_is_legal(const move_info& move) const;
	bool knight_move_is_legal(const move_info& move) const;
	bool bishop_move_is_legal(const move_info& move) const;
	bool rook_move_is_legal(const move_info& move) const;
	bool queen_move_is_legal(const move_info& move) const;
	bool king_move_is_legal(const move_info& move) const;
	bool en_passant_capture_is_legal(const move_info& move) const;
	bool castling_is_legal(const move_info& move) const;
	

	void make_normal_move(const move_info& move);
	void make_castling_move(const move_info& move);
	void make_en_passant_move(const move_info& move);
	void make_en_passant_capture(const move_info& move);
	void make_promotion_move(const move_info& move);

	void update_castling_rights();
//data members
private:
	Piece _board[8][8];

	Bitboard _white_pieces;
	Bitboard _black_pieces;

	Count _white_pieces_count[PIECE_NB / 2];
	Count _black_pieces_count[PIECE_NB / 2];

	//true - if white's move, false - black's move
	bool _white_to_play;
	//true - if the movers king is under attack
	bool _king_under_attack;
	// the file number of possible en_passant, -1 if none
	int _en_passant_file;

	// true if appropriate castling is allowed
	bool _white_left_castling;
	bool _white_right_castling;
	bool _black_left_castling;
	bool _black_right_castling;
};

}

#endif //POSITIONSTATE_H_
