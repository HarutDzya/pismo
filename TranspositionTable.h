#ifndef _TRANSPOSITION_TABLE_
#define _TRANSPOSITION_TABLE_

namespace pismo
{

class TranspositionTable
{
public:
  bool contains(const PositionState& pos, float& posValue)
  {
  }

  void push(const PositionState& pos, float posValue)
  {
    //implement Zobrist Hashing

    //purging: something like double buffering ?
  }


private:
  
};

}

#endif
