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
	
	bool contains(const PositionState& pos, eval_info& eval) const;
	
	/**
	 * override existing eval value if position is different,
	 * or position is the same but the depth is bigger
	 */
	void push(const eval_info& eval);


	void force_push(const eval_info& eval);

private:
	unsigned int hash_function(const ZobKey& zob_key) const;

private:
	std::vector<eval_info> _hash; 
};

}

#endif
