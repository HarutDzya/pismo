#include "PositionState.h"
#include "Perft.h"
#include "MemPool.h"
#include <fstream>
#include <string>
#include <cctype>
#include <ctime>
#include <iostream>

void parseInputInfo(const std::string& line, std::string& fen, uint16_t& depth, uint64_t& goldenOutput);

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Program is executed with the following command: " << argv[0] 
			<< ".exe input_file output_file" << std::endl;
	}
	else {
		std::ifstream ifStream;
		std::ofstream ofStream;
		ifStream.open(argv[1], std::ifstream::in);
		ofStream.open(argv[2], std::ofstream::out);
		if (ifStream.is_open() && ofStream.is_open()) {
			ofStream << "Output format: State\tElapsed_time\tActual_result\tExpected_result\tFEN\tDepth" << std::endl;  
			std::string line;
			pismo::MemPool::instance()->initMovesArray();
			while(std::getline(ifStream, line)) {
				std::string fen;
				uint16_t depth;
				uint64_t goldenOutput;
				parseInputInfo(line, fen, depth, goldenOutput);
				pismo::PositionState pos;
				pos.initPositionFEN(fen);
				pismo::Perft perft;
				std::time_t start = time(NULL);
				uint64_t moveCount = perft.analyze(pos, depth);
				std::time_t end = time(NULL);
				if (moveCount == goldenOutput) {
					ofStream << "PASSED: ";
				}
				else {
					ofStream << "FAILED: ";
				}
				ofStream << end - start << "sec " << moveCount 
						<< " " << goldenOutput << " " << fen << " " << depth << std::endl;
			}	
		}
		else {
			if(!ifStream.is_open()) {
				std::cout << "Cannot open the " << argv[1] << " file for reading" << std::endl;
			}
			if(!ofStream.is_open()) {
				std::cout << "Cannot open the " << argv[2] << " file for writing" << std::endl;
			}
		}
		if (ifStream.is_open()) {
			ifStream.close();
		}
		if (ofStream.is_open()) {
			ofStream.close();
		}
	}
}

void parseInputInfo(const std::string& line, std::string& fen, uint16_t& depth, uint64_t& goldenOutput)
{
	int charCount = line.size() - 1;
	while (!std::isdigit(line[charCount])) {
		--charCount;
	}
	
	std::string mcTmp;
	while (line[charCount] != ' ' && line[charCount] != '\t') {
		if (std::isdigit(line[charCount])) {
			mcTmp.push_back(line[charCount]);
		}
		--charCount;
	}
	
	goldenOutput = mcTmp[mcTmp.size() - 1] - '0';
	for (int i = mcTmp.size() - 2; i >= 0; --i) {
		goldenOutput *= 10;
		goldenOutput += (mcTmp[i] - '0');
	}

	--charCount;
	std::string dTmp;
	while (line[charCount] != ' ' && line[charCount] != '\t') {
		if (std::isdigit(line[charCount])) {
			dTmp.push_back(line[charCount]);
		}
		--charCount;
	}
	depth = dTmp[dTmp.size() - 1] - '0';
	for (int i = dTmp.size() - 2; i >= 0; --i) {
		depth *= 10;
		depth += (dTmp[i] - '0');
	}

	fen = line.substr(0, charCount);
}  
