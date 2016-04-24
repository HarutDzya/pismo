#include "Uci.h"
#include <cstdio>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "ABCore.h"
#include "PositionState.h"

namespace pismo
{
namespace UCI
{
unsigned int PROGRAM_VERSION = 1;
unsigned int MAX_COMMAND_SIZE = 100;

std::mutex searchMtx;
std::mutex stopMtx;
std::condition_variable searchCV;
std::condition_variable stopCV;
bool doSearch = false;
bool stopSearch = false;

ABCore* engine = new ABCore();
PositionState* pos = new PositionState();

enum SetOption {
	DEBUG_LOG = 0, HASH, CLEAR_HASH, UNKNOWN
};

void initUCI()
{
	std::fputs("id name Pismo ", stdout);
	std::fputc('0' + PROGRAM_VERSION, stdout);
	std::fputc('\n', stdout);
	std::fputs("id author Harut Movsisyan and Areg Ghazaryan\n\n", stdout);
	std::fputs("option name Debug Log type check defualt false\n", stdout);
	std::fputs("option name Hash type spin default 8 min 1 max 128\n", stdout);
	std::fputs("option name Clear Hash type button\n", stdout);
	std::fputs("uciok\n", stdout);
	pos->initPositionFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void manageUCI()
{
	initUCI();
	//const char* delimiter = " \t";
	char command[MAX_COMMAND_SIZE];
	while (fgets(command, MAX_COMMAND_SIZE, stdin)) {
		//char* part = std::strtok(command, delimiter);
		if (!std::strcmp(command, "isready\n")) {
			std::fputs("readyok\n", stdout);
		}
		/*else if (std::strcmp(command, "setoption")) {
			unsigned int value = 0;
			SetOption option = parseOption(value);
			switch(option) {
				case DEBUG_LOG:
					break;
				case HASH_SIZE:
					break;
				case CLEAR_HASH:
					break;
				case UNKNOWN:
					std::fputs("Unknown option:\n", stdout);
			}
		}*/
		else if (!std::strcmp(command, "go\n")) {
			std::unique_lock<std::mutex> searchLck(searchMtx);
			std::unique_lock<std::mutex> timerLck(stopMtx);
			doSearch = true;
			stopSearch = false;
			stopCV.notify_one();
			searchCV.notify_one();
		}
	}
}

void manageSearch()
{
	std::unique_lock<std::mutex> searchLck(searchMtx);
	while (true) {
		searchCV.wait(searchLck, []() {return doSearch;});
		MoveInfo move = engine->think(*pos, 8);
		printMove(move);
		doSearch = false;
	}
}

void manageTimer()
{
	std::unique_lock<std::mutex> timerLck(stopMtx);
	while (true) {
		stopCV.wait(timerLck, []() {return doSearch;});
		auto startTime = std::chrono::steady_clock::now();
		bool elapsed = false;
		while (!elapsed) {
			auto endTime = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count() 
					== 10) {
				elapsed = true;
			}
		}
		stopSearch = true;
	}
}

void printMove(const MoveInfo& move)
{
	std::fputs("bestmove ", stdout);
	if (move.from != INVALID_SQUARE) {
		std::fputc('a' + move.from % 8, stdout);
		std::fputc('1' + move.from / 8, stdout);
		std::fputc('a' + move.to % 8, stdout);
		std::fputc('1' + move.to / 8, stdout);
		std::fputs(getPromoted(move.promoted).c_str(), stdout);
	}
	else {
		std::fputs("0000", stdout);
	}
	std::fputc('\n', stdout);
}

}
}





