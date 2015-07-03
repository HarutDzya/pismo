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
	
	bool contains(const PositionState& pos, EvalInfo& eval_info) const;
	
  /**
   * override existing eval value if position is different,
   * or position is the same but the depth is bigger
   */
	void push(const EvalInfo& eval_info);


	void forcePush(const EvalInfo& eval_info);

private:
	unsigned int hash_function(const ZobKey& zob_key) const;

private:
	std::vector<EvalInfo> _hash; 
};

}

#endif