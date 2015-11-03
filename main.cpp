#include <iostream>
#include <map>
#include "PositionState.h"
#include "MoveGenerator.h"
#include "Core.h"
#include "MemPool.h"

void printBitboard(const pismo::Bitboard& board);
void printGeneratedMoves(const pismo::MovesArray& generatedMoves);

int main()
{
	using namespace pismo;
	std::map<std::string, Square> boardRep;
	for (int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			boardRep[std::string(1, ('A' + j)) + std::string(1, ('1' + i))] = (Square)(i * 8 + j);
		}
	} 

	MemPool::instance()->initMovesArray();
	PositionState pos;

	pos.initPositionFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
	pos.printBoard();
	std::cout << "Please enter: \n\tn - to make next move \n\tu - to undo the move"
		"\n\tt - for engine to think (and make a move) \n\tf - to print FEN \n\tq - to stop the game" << std::endl;
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
			pos.updateStatePinInfo();
			pos.updateSquaresUnderAttack();
			if (pos.pseudoMoveIsLegalMove(move)) {
				std::cout << "Pseudomove is legal move" << std::endl;
			}
			else {
				std::cout << "Pseudomove is illegal move" << std::endl;
			}
  		
			if (pos.moveIsLegal(move)) {
				pos.updateDirectCheckArray();
				pos.updateDiscoveredChecksInfo();
				if (pos.moveChecksOpponentKing(move)) {
					std::cout << "Move checks opponent king" << std::endl;
				}
				else {
					std::cout << "Move does not check opponent king" << std::endl;
				}
				pos.makeMove(move);
				pos.printBoard();
			}
			else {
				std::cout << "Move is illegal" << std::endl;
				//pos.printPossibleMoves(move.from);
				MovesArray possibleMoves;
				if (pos.whiteToPlay()) {
					MoveGenerator::instance()->generateWhiteMoves(pos, possibleMoves);
					printGeneratedMoves(possibleMoves);
				}
				else {
					MoveGenerator::instance()->generateBlackMoves(pos, possibleMoves);
					printGeneratedMoves(possibleMoves);
				}
			}
		}
		else if (choice == "u") {
			pos.undoMove();
			pos.printBoard();
		} 
		else if (choice == "t") {
			MoveInfo mv = p->think(pos, 5);
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

void printGeneratedMoves(const pismo::MovesArray& generatedMoves)
{
	std::cout << "Possible moves are the following" << std::endl; 
	for (std::size_t moveCount = 0; moveCount < generatedMoves.size(); ++moveCount) {
		pismo::MoveInfo move = generatedMoves[moveCount];
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
				break;
			case pismo::QUEEN_BLACK:
				std::cout << "QB" << std::endl;
				break;
			default:
				std::cout << "XP" << std::endl;
		}
	}
}


