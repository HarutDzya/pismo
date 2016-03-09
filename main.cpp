#include <iostream>
#include <map>
#include "PositionState.h"
#include "ABCore.h"
#include "MemPool.h"

void printBitboard(const pismo::Bitboard& board);

int main()
{
	using namespace pismo;
	std::map<std::string, Square> boardRep;
	for (int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			boardRep[std::string(1, ('A' + j)) + std::string(1, ('1' + i))] = (Square)(i * 8 + j);
		}
	} 

	MemPool::initMoveGenInfo();
	MemPool::initCheckPinInfo();
	PositionState pos;

	pos.initPositionFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 1");
	pos.printBoard();
	std::cout << "Please enter: \n\tn - to make next move \n\tu - to undo the move"
		"\n\tt - for engine to think (and make a move) \n\tf - to print FEN \n\tq - to stop the game" << std::endl;
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
			if(prom == "QW") {
				move.from = boardRep[sqfrom];
				move.to = boardRep[sqto];
				move.promoted = QUEEN_WHITE;
			}
			else if (prom == "QB") {
				move.from = boardRep[sqfrom];
				move.to = boardRep[sqto];
				move.promoted = QUEEN_BLACK;
			}
			else {
				move.from = boardRep[sqfrom];
				move.to = boardRep[sqto];
				move.promoted = ETY_SQUARE;
			}
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
			pos.makeMove(mv);
			pos.printBoard();
			std::cout << "Move: " <<  moveToNotation(mv) << "\n" << std::endl;
		}
    else if (choice == "f") {
      std::cout << pos.getStateFEN() << std::endl;
    }
		std::cout << "Please enter: \n\tn - to make next move, \n\tu - to undo the move"
			"\n\tt - for engine to think (and make a move) \n\tf - to print FEN \n\tq - to stop the game" << std::endl;
	}

	std::cout << pos.getStateFEN() << std::endl;	
	pos.printBoard();
	pos.printWhitePieces();
	pos.printBlackPieces();
	MemPool::destroyMoveGenInfo();
	MemPool::destroyCheckPinInfo();
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

