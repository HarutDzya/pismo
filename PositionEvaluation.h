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
  * Centi pawn (100 cp = 1 pawn) is choosen as a position evaluation granularity.
  * Intuition says light evaluation should be better choice, but everything will be clear later ...
  * For now lets only consider folowing three features:
  * - piece square table
  * - Material
  * - Mobility
  * - king safety ( hold it for later releases ?)
  *
  * returns value in centi pawns.
  * (_posValue) / 100:
  *        == -100                 - black wins (mate?)
  *        is in (-50, 0) range    - the smaller _posValue the better black postion is
  *        < -5                    - 99% GMs can win playing with black pieces (advantage of more than 5 pawns)
  *        == 0                    - position is equal
  *        > 5                     - 99% GMs can win playing with black pieces (advantage of more than 5 pawns)
  *        is in (50, 0) range     - the bigger _posValue the better white postion is
  *        == 100                  - black wins (mate?)
  */

  

  int16_t evaluate(const PositionState& pos);
  {
    evalPieceSquare();

    evalMaterial();

    evalMobility();

    //king_safety();

    return _posValue;
  }

private:
  void evalMaterial();
  
  void evalMobility();

  //position value in centi pawns
  int16_t _posValue;
};

}

#endif
