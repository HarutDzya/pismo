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
depth_array(0)
{
}

MemPool::~MemPool()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		delete depth_array[i];
	}
	delete[] depth_array;
}

void MemPool::init_moves_array()
{
	depth_array = new moves_array*[MAX_SEARCH_DEPTH];
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		depth_array[i] = new moves_array();
	}
}

moves_array& MemPool::get_moves_array(uint16_t depth)
{
	return *depth_array[depth];
}

}


