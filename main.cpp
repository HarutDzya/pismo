#include <iostream>
#include <map>
#include "PositionState.h"
#include "MoveGenerator.h"
#include "Core.h"

void print_bitboard(const pismo::Bitboard& board);
void print_generated_moves(const std::vector<pismo::move_info>& generated_moves);

int main()
{
	using namespace pismo;
	std::map<std::string, Square> board_rep;
	for (int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			board_rep[std::string(1, ('A' + j)) + std::string(1, ('1' + i))] = (Square)(i * 8 + j);
		}
	} 

	std::map<Square, std::string> square_to_notation;
	for (int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			square_to_notation[(Square)(i * 8 + j)] = std::string(1, ('A' + j)) + std::string(1, ('1' + i));
		}
	} 
	
	PositionState pos;
	/*std::vector<std::pair<Square, Piece> > pcs;
	for (int i = 0; i < 8; ++i) {
		pcs.push_back(std::pair<Square, Piece>((Square)(8 + i), PAWN_WHITE));
		pcs.push_back(std::pair<Square, Piece>((Square)(6 * 8 + i), PAWN_BLACK));
	}
	pcs.push_back(std::pair<Square, Piece>(B1, KNIGHT_WHITE));
	pcs.push_back(std::pair<Square, Piece>(G1, KNIGHT_WHITE));
	pcs.push_back(std::pair<Square, Piece>(B8, KNIGHT_BLACK));
	pcs.push_back(std::pair<Square, Piece>(G8, KNIGHT_BLACK));
	pcs.push_back(std::pair<Square, Piece>(E1, KING_WHITE));
	pcs.push_back(std::pair<Square, Piece>(E8, KING_BLACK));
	pcs.push_back(std::pair<Square, Piece>(A1, ROOK_WHITE));
	pcs.push_back(std::pair<Square, Piece>(H1, ROOK_WHITE));
	pcs.push_back(std::pair<Square, Piece>(A8, ROOK_BLACK));
	pcs.push_back(std::pair<Square, Piece>(H8, ROOK_BLACK));
	pcs.push_back(std::pair<Square, Piece>(C1, BISHOP_WHITE));
	pcs.push_back(std::pair<Square, Piece>(F1, BISHOP_WHITE));
	pcs.push_back(std::pair<Square, Piece>(C8, BISHOP_BLACK));
	pcs.push_back(std::pair<Square, Piece>(F8, BISHOP_BLACK));
	pcs.push_back(std::pair<Square, Piece>(D1, QUEEN_WHITE));
	pcs.push_back(std::pair<Square, Piece>(D8, QUEEN_BLACK));
	*/

	//pos.init_position(pcs);
	pos.init_position_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	pos.print_board();
	std::cout << "Please enter: \n\tn - to make next move \n\tu - to undo the move"
		"\n\tt - for engine to think (and make a move) \n\tq - to stop the game)" << std::endl;
	std::string choice;

 	Core* p = new Core();

	while(std::cin >> choice && choice != "q") {
		if (choice == "n") {
			std::string sqfrom;
			std::string sqto;
			std::string prom;
			std::cout << "Please enter the move" << std::endl;
			std::cin >> sqfrom >> sqto;
			std::getline(std::cin, prom);
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
  		
			if (pos.move_is_legal(move)) {
				pos.make_move(move);
				pos.print_board();
			}
			else {
				std::cout << "Move is illegal" << std::endl;
				//pos.print_possible_moves(move.from);
				std::vector<move_info> possibleMoves;
				if (pos.white_to_play()) {
					MoveGenerator::instance()->generate_white_moves(pos, possibleMoves);
					print_generated_moves(possibleMoves);
				}
				else {
					MoveGenerator::instance()->generate_black_moves(pos, possibleMoves);
					print_generated_moves(possibleMoves);
				}
			}
		}
		else if (choice == "u") {
			pos.undo_move();
			pos.print_board();
		} 
		else if (choice == "t") {
			move_info mv = p->think(pos, 5);
			pos.make_move(mv);
			pos.print_board();
			std::cout << "Move: " <<  square_to_notation[mv.from] << "->" << square_to_notation[mv.to] << std::endl;
		}
		std::cout << "Please enter: \n\tn - to make next move, \n\tu - to undo the move"
			"\n\tt - for engine to think (and make a move) \n\tq - to stop the game" << std::endl;
	}

	std::cout << pos.get_state_FEN() << std::endl;	
	pos.print_board();
	pos.print_white_pieces();
	pos.print_black_pieces();
}

void print_bitboard(const pismo::Bitboard& board)
{
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if ((board >> (i * 8 + j)) & 1) {
				std::cout << "1 ";
			}
			else {
				std::cout << "0 ";
			}
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void print_generated_moves(const std::vector<pismo::move_info>& generated_moves)
{
	std::cout << "Possible moves are the following" << std::endl; 
	for (std::size_t move_count = 0; move_count < generated_moves.size(); ++move_count) {
		pismo::move_info move = generated_moves[move_count];
		std::cout << std::string(1, ('A' + move.from % 8)) + std::string(1, ('1' + move.from / 8)) << "	";
		std::cout << std::string(1, ('A' + move.to % 8)) + std::string(1, ('1' + move.to / 8)) << "	";
		switch(move.promoted) {
			case pismo::ETY_SQUARE:
				std::cout << "ES" << std::endl;
				break;
			case pismo::KNIGHT_WHITE:
				std::cout << "NW" << std::endl;
				break;
			case pismo::BISHOP_WHITE:
				std::cout << "BW" << std::endl;
				break;
			case pismo::ROOK_WHITE:
				std::cout << "RW" << std::endl;
				break;
			case pismo::QUEEN_WHITE:
				std::cout << "QW" << std::endl;
				break;
			case pismo::KNIGHT_BLACK:
				std::cout << "NB" << std::endl;
				break;
			case pismo::BISHOP_BLACK:
				std::cout << "BB" << std::endl;
				break;
			case pismo::ROOK_BLACK:
				std::cout << "RB" << std::endl;
			case pismo::QUEEN_BLACK:
				std::cout << "QB" << std::endl;
			default:
				std::cout << "XP" << std::endl;
		}
	}
}


