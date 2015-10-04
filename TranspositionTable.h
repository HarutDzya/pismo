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
	
	bool contains(const PositionState& pos, EvalInfo& eval) const;
	
	/**
	 * override existing eval value if position is different,
	 * or position is the same but the depth is bigger
	 */
	void push(const EvalInfo& eval);


	void forcePush(const EvalInfo& eval);

private:
	unsigned int hashFunction(const ZobKey& zobKey) const;

private:
	std::vector<EvalInfo> _hash; 
};

}

#endif
