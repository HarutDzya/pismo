#include "MemPool.h"
#include "MoveGenerator.h"

namespace pismo
{

MoveGenInfo* g_moveGenInfo[MAX_SEARCH_DEPTH];
MoveGenInfo* g_quiescenceMoveGenInfo[MAX_QUIESCENCE_SEARCH_DEPTH];

CheckPinInfo* g_checkPinInfo[MAX_SEARCH_DEPTH];
CheckPinInfo* g_quiescenceCheckPinInfo[MAX_QUIESCENCE_SEARCH_DEPTH];

void MemPool::initMoveGenInfo() 
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		g_moveGenInfo[i] = new MoveGenInfo();
	}

	for (uint16_t i = 0; i < MAX_QUIESCENCE_SEARCH_DEPTH; ++i) {
		g_quiescenceMoveGenInfo[i] = new MoveGenInfo();
	}
}

void MemPool::destroyMoveGenInfo()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		delete g_moveGenInfo[i];
	}

	for (uint16_t i = 0; i < MAX_QUIESCENCE_SEARCH_DEPTH; ++i) {
		delete g_quiescenceMoveGenInfo[i];
	}

}

void MemPool::initCheckPinInfo()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		g_checkPinInfo[i] = new CheckPinInfo();
	}

	for (uint16_t i = 0; i < MAX_QUIESCENCE_SEARCH_DEPTH; ++i) {
		g_quiescenceCheckPinInfo[i] = new CheckPinInfo();
	}
}

void MemPool::destroyCheckPinInfo()
{
	for (uint16_t i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		delete g_checkPinInfo[i];
	}

	for (uint16_t i = 0; i < MAX_QUIESCENCE_SEARCH_DEPTH; ++i) {
		delete g_quiescenceCheckPinInfo[i];
	}
}

MoveGenInfo* MemPool::getMoveGenInfo(uint16_t depth)
{
	return g_moveGenInfo[depth];
}

MoveGenInfo* MemPool::getQuiescenceMoveGenInfo(uint16_t depth)
{
	return g_quiescenceMoveGenInfo[depth];
}

CheckPinInfo* MemPool::getCheckPinInfo(uint16_t depth)
{
	return g_checkPinInfo[depth];
}

CheckPinInfo* MemPool::getQuiescenceCheckPinInfo(uint16_t depth)
{
	return g_quiescenceCheckPinInfo[depth];
}

}
