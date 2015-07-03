#ifndef _POSITION_EVALUATION_
#define _POSITION_EVALUATION_

namespace pismo
{
/**
 * https://chessprogramming.wikispaces.com/Evaluation
 */

class PositionEvaluation
{
public:

  /**
  *
  * Intuition says light evaluation should be better choice, but everything will be clear later ...
  * For now lets only consider folowing three features:
  * - Material
  * - Mobility
  * - king safety ( hold it for later releases ?)
  *
  * TODO: Piece-Square Tables (TODO: later releases)
  * 
  *
  * return value (_posValue):
  * _posValue == -1000                - black wins (mate?)
  * _posValue is in (-50, 0) range    - the smaller _posValue the better black postion is
  * _posValue < -5                    - 99% GMs can win playing with black pieces
  * _posValue == 0                    - position is equal
  * _posValue > 5                     - 99% GMs can win playing with black pieces
  * _posValue is in (50, 0) range     - the bigger _posValue the better white postion is
  * _posValue == 1000                 - black wins (mate?)
  */

  

  float evaluate(const PositionState& pos);
  {
    eval_material();

    eval_mobility();

    //king_safety();
  }

private:
  void eval_material();
  
  void eval_mobility();

  float _pos_value;
};

}

#endif
