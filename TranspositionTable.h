#ifndef TRANSPOSITION_TABLE_
#define TRANSPOSITION_TABLE_

#include "utils.h"
#include <vector>

namespace pismo
{
class PositionState;
const unsigned int HASH_TABLE_SIZE = 1000000;

class TranspositionTable
{
public:
	TranspositionTable();
	
	bool contains(const PositionState& pos, evalInfo& eval) const;
	
	/**
	 * override existing eval value if position is different,
	 * or position is the same but the depth is bigger
	 */
	void push(const evalInfo& eval);


	void forcePush(const evalInfo& eval);

private:
	unsigned int hashFunction(const ZobKey& zobKey) const;

private:
	std::vector<evalInfo> _hash; 
};

}

#endif
