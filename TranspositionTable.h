#ifndef TRANSPOSITION_TABLE_
#define TRANSPOSITION_TABLE_

#include "utils.h"
#include <vector>

namespace pismo
{
class PositionState;
struct EvalInfo
{
	int16_t pos_value;
	ZobKey zob_key;
	Count depth;
  EvalInfo(int16_t v, ZobKey z, Count d)
  : pos_value(v),
    zob_key(z),
    depth(d)
  {
  }

	//TODO:Add later best move
};

const Count HASH_TABLE_SIZE = 1000000;

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
	Count hash_function(const ZobKey& zob_key) const;

private:
	std::vector<EvalInfo> _hash; 
};

}

#endif
