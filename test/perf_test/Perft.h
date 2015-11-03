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
	uint64_t analyze(PositionState& pos, uint16_t depth, bool begin = false) const;

	Perft();

	~Perft();

private:

	Perft(const Perft&); //non-copy constructable
	Perft& operator=(const Perft&); //non-assignable

};

}

#endif //PERFT_H_

