#ifndef _POSITION_EVALUATION_
#define _POSITION_EVALUATION_

#include "utils.h"
#include <vector>

namespace pismo
{
class PositionState;
const unsigned int MATERIAL_HASH_SIZE = 10000;

/**
 * https://chessprogramming.wikispaces.com/Evaluation
 */

class PositionEvaluation
{
public:

	/*
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
	 *        == 0                    - position is equa
	 *        > 5                     - 99% GMs can win playing with black pieces (advantage of more than 5 pawns)
	 *        is in (50, 0) range     - the bigger _posValue the better white postion is
	 *        == 100                  - black wins (mate?)
	 */
  
	PositionEvaluation();

	int16_t evaluate(const PositionState& pos);
  

private:
	
	void evalMaterial(const PositionState& pos);
	void evalPieceSquare(const PositionState& pos);  
	void evalMobility(const PositionState& pos);

	unsigned int hashFunction(const ZobKey& materialZobKey) const;

	struct materialInfo {
		int16_t materialValue;
		ZobKey materialZobKey;

		materialInfo(int16_t mv = 0, ZobKey mz = 0)
		: materialValue(mv),
		materialZobKey(mz)
	 	{
	 	}
	};

	//position value in centi pawns
	int16_t _posValue;

	std::vector<materialInfo> _materialHash;
};

}

#endif
