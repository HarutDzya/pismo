#include "TranspositionTable.h"
#include "PositionState.h"

namespace pismo
{

TranspositionTable::TranspositionTable():
_hash(HASH_TABLE_SIZE)
{
}

bool TranspositionTable::contains(const PositionState& pos, EvalInfo& eval_info) const
{
	Count index = hash_function(pos.get_state_zob_key());
	if (_hash[index].zob_key == pos.get_state_zob_key()) {
		eval_info = _hash[index];
		return true;
	}
	else {
		return false;
	}
}

void TranspositionTable::push(const EvalInfo& eval_info)
{
	Count index = hash_function(eval_info.zob_key);
	if ((_hash[index].zob_key != eval_info.zob_key) || (_hash[index].depth < eval_info.depth)) {
		_hash[index] = eval_info;
	}
}

Count TranspositionTable::hash_function(const ZobKey& zob_key) const
{
	return (zob_key % HASH_TABLE_SIZE);
}
}
