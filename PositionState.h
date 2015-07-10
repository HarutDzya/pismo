#ifndef POSITIONSTATE_H_
#define POSITIONSTATE_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class BitboardImpl;
class ZobKeyImpl;

class PositionState
{
public:
	PositionState();
	~PositionState();

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
	void make_move(const move_info& move);

	/*
	Makes an undo move of the last made move, by reverting all
	state variables to the previous state
	*/
	void undo_move();

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

	/*
	Prints possible moves from Square from on the board
	*/
	void print_possible_moves(Square from) const;

	ZobKey get_zob_key() const {return _zob_key;}

	Piece const (&get_board()const)[8][8] {return _board;}

	unsigned int get_piece_count(Piece p) const {return _piece_count[p];}

	int get_material_zob_key() const {return _material_zob_key;}

	int get_pst_value() const {return _pst_value;}

	bool white_to_play() const {return _white_to_play;}

//private member functions
private:
	void set_piece(Square s, Piece p);
	bool init_position_is_valid(const std::vector<std::pair<Square, Piece> >& pieces) const;
	bool pawn_move_is_legal(const move_info& move, bool& is_en_passant_capture) const;
	bool knight_move_is_legal(const move_info& move) const;
	bool bishop_move_is_legal(const move_info& move) const;
	bool rook_move_is_legal(const move_info& move) const;
	bool queen_move_is_legal(const move_info& move) const;
	bool king_move_is_legal(const move_info& move) const;
	bool en_passant_capture_is_legal(const move_info& move) const;
	bool castling_is_legal(const move_info& move) const;
	
	Bitboard squares_under_attack(Color attacked_color) const;

	void make_lazy_move(const move_info& move, bool is_en_passant_capture, Piece& captured_piece) const;
	void undo_lazy_move(const move_info& move, bool is_en_passant_capture, Piece captured_piece) const;

	void make_normal_move(const move_info& move);
	void make_castling_move(const move_info& move);
	void make_en_passant_move(const move_info& move);
	void make_en_passant_capture(const move_info& move);
	void make_promotion_move(const move_info& move);

	void update_castling_rights();

	void add_piece_to_bitboards(Square sq, Color clr);
	void remove_piece_from_bitboards(Square sq, Color clr);

	int calculate_pst_value(Piece p, Square s) const;
	void update_game_status();

	void undo_normal_move(const undo_move_info& move);
	void undo_castling_move(const undo_move_info& move);
	void undo_en_passant_move(const undo_move_info& move);
	void undo_en_passant_capture(const undo_move_info& move);
	void undo_promotion_move(const undo_move_info& move);

	struct undo_move_info {
		Square from;
		Square to;
		Piece moved_piece;
		Piece captured_piece;
		int8_t en_passant_file;
		uint8_t castling_rights;
		bool is_en_pass_capture;
	};


	class MoveStack {
		public:
			MoveStack();
			void push(const undo_move_info& move);
			undo_move_info pop();
			bool isEmpty() const;
			uint32_t get_size() const;
		
		private:
			struct node {
				undo_move_info move;
				node* next;

				node(const undo_move_info& m, node* const nt = 0)
				: move(m),
				next(nt)
				{
				}
			};

			node* _first;
			uint32_t _stack_size;
	};


//data members
private:
	Piece _board[8][8];
	
	// Occupation bitboards (4 for each color)
	Bitboard _white_pieces;
	Bitboard _white_pieces_transpose;
	Bitboard _white_pieces_diag_a1h8;
	Bitboard _white_pieces_diag_a8h1;
	Bitboard _black_pieces;
	Bitboard _black_pieces_transpose;
	Bitboard _black_pieces_diag_a1h8;
	Bitboard _black_pieces_diag_a8h1;

	// Each memeber of the array shows the number of appropriate 
	// piece available 
	unsigned int _piece_count[PIECE_NB];

	const BitboardImpl* _bitboard_impl;
	const ZobKeyImpl* _zob_key_impl;

	//true - if white's move, false - black's move
	bool _white_to_play;
	// the file number of possible en_passant, -1 if none
	int _en_passant_file;
	
	//Shows white and black king position after the move
	Square _white_king_position;
	Square _black_king_position;

	// true if appropriate castling is allowed
	bool _white_left_castling;
	bool _white_right_castling;
	bool _black_left_castling;
	bool _black_right_castling;

	// true if it is a middle game
	bool _is_middle_game;

	// Zobrist key for the state of the game
	ZobKey _zob_key;
	
	// Zobrist key for material of the game
	ZobKey _material_zob_key;

	// Piece Square Table value for the state of the game
	int _pst_value;

	MoveStack _move_stack;
};

}

#endif //POSITIONSTATE_H_
