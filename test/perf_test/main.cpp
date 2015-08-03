#include "PositionState.h"
#include "Perft.h"
#include <fstream>
#include <string>
#include <cctype>
#include <ctime>
#include <iostream>

void parse_input_info(const std::string& line, std::string& fen, uint16_t& depth, uint64_t& golden_output);

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Program is executed with the following command: " << argv[0] 
			<< ".exe input_file output_file" << std::endl;
	}
	else {
		std::ifstream if_stream;
		std::ofstream of_stream;
		if_stream.open(argv[1], std::ifstream::in);
		of_stream.open(argv[2], std::ofstream::out);
		if (if_stream.is_open() && of_stream.is_open()) {
			of_stream << "Output format: State\tElapsed_time\tActual_result\tExpected_result\tFEN\tDepth" << std::endl;  
			std::string line;
			while(std::getline(if_stream, line)) {
				std::string fen;
				uint16_t depth;
				uint64_t golden_output;
				parse_input_info(line, fen, depth, golden_output);
				pismo::PositionState pos;
				pos.init_position_FEN(fen);
				pismo::Perft perft;
				std::time_t start = time(NULL);
				uint64_t move_count = perft.analyze(pos, depth);
				std::time_t end = time(NULL);
				if (move_count == golden_output) {
					of_stream << "PASSED: ";
				}
				else {
					of_stream << "FAILED: ";
				}
				of_stream << end - start << "sec " << move_count 
						<< " " << golden_output << " " << fen << " " << depth << std::endl;
			}	
		}
		else {
			if(!if_stream.is_open()) {
				std::cout << "Cannot open the file for reading" << argv[1] << std::endl;
			}
			if(!of_stream.is_open()) {
				std::cout << "Cannot open the file for writing" << argv[2] << std::endl;
			}
		}
		if (if_stream.is_open()) {
			if_stream.close();
		}
		if (of_stream.is_open()) {
			of_stream.close();
		}
	}
}

void parse_input_info(const std::string& line, std::string& fen, uint16_t& depth, uint64_t& golden_output)
{
	int char_count = line.size() - 1;
	while (!std::isdigit(line[char_count])) {
		--char_count;
	}
	
	std::string mc_tmp;
	while (line[char_count] != ' ' && line[char_count] != '\t') {
		if (std::isdigit(line[char_count])) {
			mc_tmp.push_back(line[char_count]);
		}
		--char_count;
	}
	
	golden_output = mc_tmp[mc_tmp.size() - 1] - '0';
	for (int i = mc_tmp.size() - 2; i >= 0; --i) {
		golden_output *= 10;
		golden_output += (mc_tmp[i] - '0');
	}

	--char_count;
	std::string d_tmp;
	while (line[char_count] != ' ' && line[char_count] != '\t') {
		if (std::isdigit(line[char_count])) {
			d_tmp.push_back(line[char_count]);
		}
		--char_count;
	}
	depth = d_tmp[d_tmp.size() - 1] - '0';
	for (int i = d_tmp.size() - 2; i >= 0; --i) {
		depth *= 10;
		depth += (d_tmp[i] - '0');
	}

	fen = line.substr(0, char_count);
}  
