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


	set_piece(Square s, Piece p);

	init_position(<std::vector<std::pair<Square, Piece> > pieces); 


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
