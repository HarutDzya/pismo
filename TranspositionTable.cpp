#include "TranspositionTable.h"
#include "PositionState.h"

namespace pismo
{

TranspositionTable::TranspositionTable():
_hash(HASH_TABLE_SIZE)
{
}

bool TranspositionTable::contains(const PositionState& pos, eval_info& eval) const
{
	unsigned int index = hash_function(pos.get_zob_key());
	if (_hash[index].zob_key == pos.get_zob_key()) {
		eval = _hash[index];
		return true;
	}
	else {
		return false;
	}
}

void TranspositionTable::push(const eval_info& eval)
{
	unsigned int index = hash_function(eval.zob_key);
	if ((_hash[index].zob_key != eval.zob_key) || (_hash[index].depth < eval.depth)) {
		_hash[index] = eval;
	}
}

void TranspositionTable::force_push(const eval_info& eval)
{
	_hash[hash_function(eval.zob_key)] = eval;
}

unsigned int TranspositionTable::hash_function(const ZobKey& zob_key) const
{
	return (zob_key % HASH_TABLE_SIZE);
	//TODO:Change the impl to return first log(HASH_TABLE_SIZE) + 1 bits of zob_key
}
}
