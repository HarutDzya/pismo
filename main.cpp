#include <iostream>
#include <map>
#include "PositionState.h"
#include "ABCore.h"
#include "MemPool.h"
#include "Uci.h"
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void printBitboard(const pismo::Bitboard& board);
pismo::Piece getPromoted(const std::string& piece);


const unsigned int MAX_COMMAND_SIZE = 100;

int main()
{
	using namespace pismo;
	char command[MAX_COMMAND_SIZE];
	while (std::fgets(command, MAX_COMMAND_SIZE, stdin)) {
		if (!std::strcmp(command, "uci\n")) {
		std::thread searchThread(UCI::manageSearch);
		std::thread timerThread(UCI::manageTimer);
		UCI::manageUCI();
		searchThread.join();
		timerThread.join();
		}
		else if (!std::strcmp(command, "nouci\n")) {
			std::map<std::string, Square> boardRep;
			for (int i = 0; i < 8; ++i) {
				for(int j = 0; j < 8; ++j) {
					boardRep[std::string(1, ('A' + j)) + std::string(1, ('1' + i))] = (Square)(i * 8 + j);
				}
			}
			
			MemPool::initMoveGenInfo();
			MemPool::initCheckPinInfo();
			PositionState pos;
			
			pos.initPositionFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			pos.printBoard();
			std::cout << "Please enter: \n\tn - to make next move \n\tu - to undo the move"
				" \n\tt - for engine to think (and make a move) \n\tf - to print FEN \n\tq - to stop the game" << std::endl;
			std::string choice;

			ABCore* p = new ABCore();

			while(std::cin >> choice && choice != "q") {
				if (choice == "n") {
					std::string sqfrom;
					std::string sqto;
					std::string prom;
					std::cout << "Please enter the move" << std::endl;
					std::cin >> sqfrom >> sqto;
					std::getline(std::cin, prom);
					prom.erase(0, 1);
					MoveInfo move;
					move.from = boardRep[sqfrom];
					move.to = boardRep[sqto];
					move.promoted = getPromoted(prom);
					pos.updateMoveType(move);
					pos.initCheckPinInfo(0);
					pos.makeMove(move);
					pos.printBoard();
				}
				else if (choice == "u") {
					pos.undoMove();
					pos.printBoard();
				}
				else if (choice == "t") {
					MoveInfo mv = p->think(pos, 6);
					if (mv.from != INVALID_SQUARE) {
						pos.makeMove(mv);
						pos.printBoard();
						std::cout << "Move: " <<  moveToNotation(mv) << "\n" << std::endl;
					}
					else {
						std::cout << "Game over: there is no more moves" << std::endl;
						break;
					}
				}
				else if (choice == "f") {
					std::cout << pos.getStateFEN() << std::endl;
				}
				std::cout << "Please enter: \n\tn - to make next move, \n\tu - to undo the move"
					" \n\tt - for engine to think (and make a move) \n\tf - to print FEN \n\tq - to stop the game" << std::endl;
			}

			std::cout << pos.getStateFEN() << std::endl;
			pos.printBoard();
			pos.printWhitePieces();
			pos.printBlackPieces();
			MemPool::destroyMoveGenInfo();
			MemPool::destroyCheckPinInfo();
			break;
		}
		else if (!std::strcmp(command, "quit\n")) {
				std::exit(EXIT_SUCCESS);
		}
	}
}

void printBitboard(const pismo::Bitboard& board)
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

pismo::Piece getPromoted(const std::string& piece)
{
	if (piece == "QW") {
		return pismo::QUEEN_WHITE;
	}
	else if (piece == "QB") {
		return pismo::QUEEN_BLACK;
	}
	else if (piece == "RW") {
		return pismo::ROOK_WHITE;
	}
	else if (piece == "RB") {
		return pismo::ROOK_BLACK;
	}
	else if (piece == "BW") {
		return pismo::BISHOP_WHITE;
	}
	else if (piece == "BB") {
		return pismo::BISHOP_BLACK;
	}
	else if (piece == "NW") {
		return pismo::KNIGHT_WHITE;
	}
	else if (piece == "NB") {
		return pismo::KNIGHT_BLACK;
	}
	
	return pismo::ETY_SQUARE;
}
