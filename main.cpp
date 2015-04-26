#include <iostream>
#include <map>
#include "PositionState.h"


int main()
{
	using namespace pismo;
	std::map<std::string, Square> board_rep;
	for (int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			board_rep[std::string(1, ('A' + j)) + std::string(1, ('1' + i))] = (Square)(i * 8 + j);
		}
	} 
	
	PositionState pos;
	std::vector<std::pair<Square, Piece> > pcs;
	for (int i = 0; i < 8; ++i) {
		pcs.push_back(std::pair<Square, Piece>((Square)(8 + i), PAWN_WHITE));
		pcs.push_back(std::pair<Square, Piece>((Square)(6 * 8 + i), PAWN_BLACK));
	}
	pos.init_position(pcs);
	pos.print_board();
	std::cout << "Please enter next move (q to stop the game)" << std::endl;
	std::string sqfrom;
	std::string sqto;
	std::string prom;
	while (std::cin >> sqfrom && sqfrom != "q" && std::cin >> sqto && std::cin >> prom) {
		move_info move;
		if(prom == "QW") {
			move.from = board_rep[sqfrom];
			move.to = board_rep[sqto];
			move.promoted = QUEEN_WHITE;
		}
		else if (prom == "QB") {
			move.from = board_rep[sqfrom];
			move.to = board_rep[sqto];
			move.promoted = QUEEN_BLACK;
		}
		else {
			move.from = board_rep[sqfrom];
			move.to = board_rep[sqto];
			move.promoted = ETY_SQUARE;
		}
	
  		if (pos.make_move(move)) {
			pos.print_board();
			std::cout << "Please enter next move (q to stop the game)" << std::endl;
		}
		else {
			std::cout << "Move is illegal" << std::endl;
			std::cout << "Please enter next move (q to stop the game)" << std::endl;
		}
	}
	
	pos.print_board();
	pos.print_white_pieces();
	pos.print_black_pieces();
}
