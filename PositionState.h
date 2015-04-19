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
	Checks only the presence of the piece in Square from, whether it 
	is that peice turn and the emptiness of the Square to
	*/
	bool move_is_legal(Square from, Square to) const;
	
	/*
	Makes a move if the move if legal according to the move_is_legal
	method
	*/
	void move(Square from, Square to);
	
	/*
	Checks all the rules of the game for legality of the move
	even if it involves capture of the opponent piece
	*/
	bool move_is_legal_full(Square from, Square to) const;
	
	/*
	Prints two boards for white and black pieces seperately
	using information from whitePieces Bitboard and blackPieces
	Bitboard
	*/
	void print_double() const;

	/*
	Prints single board for both pieces using information
	from board array
	*/
	void print_single() const;

//data members
private:
	Piece board[8][8];

	Bitboard whitePieces;
	Bitboard blackPieces;

	//true - if white's move, false - black's move
	bool whiteToPlay;
	//true - if the movers king is under attack
	bool kingUnderAttack;
};

}

#endif //POSITIONSTATE_H_
