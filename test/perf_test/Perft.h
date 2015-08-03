#ifndef PERFT_H_
#define PERFT_H_

#include "utils.h"

namespace pismo
{
class PositionState;

class Perft
{
public:
	/*
	 * Counts the leaf nodes of the game for PositionState
	 * pos at the depth
	 */
	uint64_t analyze(PositionState& pos, uint16_t depth) const;

	Perft();

	~Perft();

private:

	void count_moves(PositionState& pos, uint16_t depth, uint64_t& move_count) const;


	Perft(const Perft&); //non-copy constructable
	Perft& operator=(const Perft&); //non-assignable

};

}

#endif //PERFT_H_

