#ifndef POSITION_H_
#define POSITION_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class Position
{
public:
	Position();


	void set_piece(Square s, Piece p);

	void init_position(const std::vector<std::pair<Square, Piece> >& pieces); 
	bool move_is_legal(Square from, Square to) const;
	void move(Square from, Square to);
	void print() const;


//data members
private:
	Piece board[8][8];

	Bitboard whitePieces;
	Bitboard blackPieces;

	//true - if white's move, false - black's move
	bool whiteToPlay;
};

}

#endif //POSITION_H_
