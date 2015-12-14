#include "MemPool.h"
#include "MoveGenerator.h"

namespace pismo
{

MoveGenInfo* g_moveGenInfo[MAX_SEARCH_DEPTH];

void MemPool::initMoveGenInfo() 
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		g_moveGenInfo[i] = new MoveGenInfo();
	}
}

void MemPool::destroyMoveGenInfo()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		delete g_moveGenInfo[i];
	}
}

MoveGenInfo& MemPool::getMoveGenInfo(uint16_t depth)
{
	return g_moveGenInfo[i];
}

}


