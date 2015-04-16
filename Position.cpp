#include "Position.h"

namespace pismo
{

Position::Position() : whiteToPlay(true)
{
}

void Position::set_piece(Square s, Piece p)
{
	board[s / 8][s % 8] = p;
	Bitboard tmp = 1;
	if(p <= KING_WHITE)
		whitePieces = whitePieces | (tmp << s);
	else
		blackPieces = blackPieces | (tmp << s);
}

void Position::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	for(std::size_t i = 0; i < pieces.size(); ++i)
		set_piece(pieces[i].first, pieces[i].second);
}
}
