#include "MemPool.h"

namespace pismo
{
MemPool* MemPool::_instance = 0;

MemPool* MemPool::instance()
{
	if (!_instance) {
		_instance = new MemPool();
	}

	return _instance;
}

void MemPool::destroy()
{
	delete _instance;
	_instance = 0;
}

MemPool::MemPool():
depthArray(0)
{
}

MemPool::~MemPool()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		delete depthArray[i];
	}
	delete[] depthArray;
}

void MemPool::initMovesArray()
{
	depthArray = new MovesArray*[MAX_SEARCH_DEPTH];
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		depthArray[i] = new MovesArray();
	}
}

MovesArray& MemPool::getMovesArray(uint16_t depth)
{
	return *depthArray[depth];
}

}


