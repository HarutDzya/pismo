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
	 * (_pos_value) / 100:
	 *        == -100                 - black wins (mate?)
	 *        is in (-50, 0) range    - the smaller _pos_value the better black postion is
	 *        < -5                    - 99% GMs can win playing with black pieces (advantage of more than 5 pawns)
	 *        == 0                    - position is equa
	 *        > 5                     - 99% GMs can win playing with black pieces (advantage of more than 5 pawns)
	 *        is in (50, 0) range     - the bigger _pos_value the better white postion is
	 *        == 100                  - black wins (mate?)
	 */
  
	PositionEvaluation();

	int16_t evaluate(const PositionState& pos);
  

private:
	
	void eval_material(const PositionState& pos);
	void eval_piece_square(const PositionState& pos);  
	void eval_mobility(const PositionState& pos);

	unsigned int hash_function(const ZobKey& material_zob_key) const;

	struct material_info {
		int16_t material_value;
		ZobKey material_zob_key;

		material_info(int16_t mv = 0, ZobKey mz = 0)
		: material_value(mv),
		material_zob_key(mz)
	 	{
	 	}
	};

	//position value in centi pawns
	int16_t _pos_value;

	std::vector<material_info> _material_hash;
};

}

#endif
