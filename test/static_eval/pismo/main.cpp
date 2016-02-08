#include "PositionState.h"
#include "PositionEvaluation.h"
#include <cstring>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
	std::string fen;
	if (argc == 9) {
		if (std::strcmp(argv[1], "position") == 0 && std::strcmp(argv[2], "fen") == 0) {
			for (int argIndex = 3; argIndex < argc; ++ argIndex) {
				fen.append(argv[argIndex]);
				fen.append(" ");
			}
		}
	}
	else if (std::getline(std::cin, fen)) {
	   if (fen.compare(0, 13, "position fen ") == 0) {
		   fen.erase(0, 13);
	   }
	}
	if (fen.size() != 0) {
		pismo::PositionState pos;
		pos.initPositionFEN(fen);
		pismo::PositionEvaluation posEval;
		posEval.initPosEval();
		std::cout << posEval.evaluate(pos) / 100.00 << std::endl;
	}

	return 0;
}

