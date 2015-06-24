#ifndef TRANSPOSITION_TABLE_
#define TRANSPOSITION_TABLE_

#include "utils.h"
#include <vector>

namespace pismo
{
class PositionState;
struct EvalInfo
{
	float pos_value;
	ZobKey zob_key;
	Count depth;
	//TODO:Add later best move
};

const Count HASH_TABLE_SIZE = 1000000;

class TranspositionTable
{
public:
	TranspositionTable();
	
	bool contains(const PositionState& pos, EvalInfo& eval_info) const;
	
	void push(const EvalInfo& eval_info);

private:
	Count hash_function(const ZobKey& zob_key) const;

private:
	std::vector<EvalInfo> _hash; 
};

}

#endif
