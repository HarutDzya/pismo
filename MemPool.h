#ifndef _MEM_POOL_
#define _MEM_POOL_

#include "utils.h"
#include "assert.h"

namespace pismo
{

//reusing allocated memory to avoid wasting time on system calls (free(), alloc())


//maximum number of moves one side can have in current position
const int MAX_POSSIBLE_MOVES = 100;


const int MAX_SEARCH_DEPTH = 60;

struct movesArray
{
	moveInfo* arr;
	int _size;
	movesArray() : _size(0)
	{
		arr = new moveInfo[MAX_POSSIBLE_MOVES];
	}

	~movesArray()
	{
		delete[] arr;
	}

	void push_back(moveInfo& move)
	{
		assert(_size != MAX_POSSIBLE_MOVES);
		arr[_size++] = move;
	}

	void clear()
	{
		_size = 0;
	}

	const moveInfo& operator[](uint16_t i) const
	{ 
		assert(i < _size);
		return arr[i];
	}

	uint16_t size() const
	{
		return _size;
	}

	bool empty() const
	{
		return _size == 0;
	}
};

class MemPool 
{
public:
	static MemPool* instance();
	static void destroy();
	
	//call this funciton at the beginning of program
	void initMovesArray();

	movesArray& getMovesArray(uint16_t depth);

private:
	MemPool();
	~MemPool();


	MemPool(const MemPool&); //non-copyable
	MemPool& operator=(const MemPool&); //non-assignable

private:
	static MemPool* _instance;
	movesArray** depthArray;
};

}

#endif
