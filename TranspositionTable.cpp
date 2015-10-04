#include "TranspositionTable.h"
#include "PositionState.h"

namespace pismo
{

TranspositionTable::TranspositionTable():
_hash(HASH_TABLE_SIZE)
{
}

bool TranspositionTable::contains(const PositionState& pos, EvalInfo& eval) const
{
	unsigned int index = hashFunction(pos.getZobKey());
	if (_hash[index].zobKey == pos.getZobKey()) {
		eval = _hash[index];
		return true;
	}
	else {
		return false;
	}
}

void TranspositionTable::push(const EvalInfo& eval)
{
	unsigned int index = hashFunction(eval.zobKey);
	if ((_hash[index].zobKey != eval.zobKey) || (_hash[index].depth < eval.depth)) {
		_hash[index] = eval;
	}
}

void TranspositionTable::forcePush(const EvalInfo& eval)
{
	_hash[hashFunction(eval.zobKey)] = eval;
}

unsigned int TranspositionTable::hashFunction(const ZobKey& zobKey) const
{
	return (zobKey % HASH_TABLE_SIZE);
	//TODO:Change the impl to return first log(HASH_TABLE_SIZE) + 1 bits of zobKey
}
}
