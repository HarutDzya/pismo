#ifndef _POSITION_EVALUATION_
#define _POSITION_EVALUATION_

namespace pismo
{

const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;

const int LEGAL_MOVE_VALUE = 10;

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
    evalMaterial();

    evalMobility();

    //king_safety();
  }

private:
  void evalMaterial();
  
  void evalMobility();

  float _posValue;

  unsigned int _prev_moves_count;
};

}

#endif
