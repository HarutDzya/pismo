#include <iostream>
#include "PositionState.h"


int main()
{
	using namespace pismo;
	PositionState pos;
	std::vector<std::pair<Square, Piece> > pcs;
	pcs.push_back(std::pair<Square, Piece>(C2, BISHOP_WHITE));
	pcs.push_back(std::pair<Square, Piece>(B5, KING_BLACK));
	pos.init_position(pcs);
	pos.print_single();
	pos.move(C3, C6);
	pos.move(B5, C2);
	pos.move(C2, H5);
	pos.move(B5, A2);
	pos.print_single();
}
