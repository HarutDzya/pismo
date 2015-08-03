#include "PositionState.h"
#include "Perft.h"
#include <fstream>
#include <string>
#include <cctype>

void parse_input_info(const std::string& line, std::string& fen, uint16_t& depth, uint64_t& golden_output);
void append_failed_info(std::string& failed_results, const std::string& line, uint64_t move_count);

int main(int argc, char* argv[])
{
	std::ifstream if_stream;
	std::ofstream of_stream;
	of_stream.open(argv[argc - 1], std::ofstream::out);
	for (int i = 1; i < argc - 1; ++i) {
		if_stream.open(argv[i], std::ifstream::in);
		if (if_stream.is_open()) {
			std::string line;
			std::string failed_results;
			unsigned int test_count = 0;
			unsigned int passed_test_count = 0;
			while(std::getline(if_stream, line)) {
				std::string fen;
				uint16_t depth;
				uint64_t golden_output;
				parse_input_info(line, fen, depth, golden_output);
				pismo::PositionState pos;
				pos.init_position_FEN(fen);
				pismo::Perft perft;
				uint64_t move_count = perft.analyze(pos, depth);
				++test_count;
				if (move_count == golden_output) {
					++passed_test_count;
				}
				else {
					append_failed_info(failed_results, line, move_count);
				}
			}
			if (of_stream.is_open()) {
				of_stream << "Output for file " << argv[i] << ":" << std::endl;
				of_stream << "From " << test_count << " tests " << passed_test_count << " passed." << std::endl;
				if (!failed_results.empty()) {
					of_stream << "Failed tests are (format: input actual_result):" << std::endl;
					of_stream << failed_results;
				}
			}	
		}
		if_stream.close();
		if_stream.clear();
	}
	of_stream.close();
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

void append_failed_info(std::string& failed_results, const std::string& line, uint64_t move_count)
{
	unsigned int line_end = line.size() - 1;
	while (!std::isdigit(line[line_end])) {
	       --line_end;
	}	       
	failed_results.append(line.substr(0, line_end + 1));
	failed_results.push_back(' ');
	std::string mc_rev;
	do {
		mc_rev.push_back('0' + move_count % 10);
		move_count /= 10;
	} while (move_count != 0);
	
	for(int i = mc_rev.size() - 1; i >=0; --i) {
		failed_results.push_back(mc_rev[i]);
	}
	failed_results.push_back('\n');
}
	
