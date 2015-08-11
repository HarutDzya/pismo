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

struct moves_array
{
	move_info* arr;
	int _size;
	moves_array() : _size(0)
	{
		arr = new move_info[MAX_POSSIBLE_MOVES];
	}

	~moves_array()
	{
		delete[] arr;
	}

	void push_back(move_info& move)
	{
		assert(_size != MAX_POSSIBLE_MOVES);
		arr[_size++] = move;
	}

	void clear()
	{
		_size = 0;
	}

	const move_info& operator[](uint16_t i) const
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
	void init_moves_array();

	moves_array& get_moves_array(uint16_t depth);

private:
	MemPool();
	~MemPool();


	MemPool(const MemPool&); //non-copyable
	MemPool& operator=(const MemPool&); //non-assignable

private:
	static MemPool* _instance;
	moves_array** depth_array;
};

}

#endif
