#ifndef _POSITION_EVALUATION_
#define _POSITION_EVALUATION_

#include "utils.h"
#include <vector>

namespace pismo
{
class PositionState;

struct MaterialInfo
{
	int16_t value;
	// TODO: Later add aditional things
};

const unsigned int MATERIAL_TABLE_SIZE = 2 * 2 * 3 * 3 * 3 * 3 * 3 * 3 * 9 * 9; 

extern const uint16_t materialPieceIndex[PIECE_NB];
extern const uint8_t materialMaxUsual[PIECE_NB];
extern const uint16_t materialToFlag[PIECE_NB];

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
	~PositionEvaluation();

	int16_t evaluate(const PositionState& pos);
  

private:
	// Initializes material table to precomputed
	// values using materialPieceIndex to compute
	// index
	void initMaterialTable();
	
	void evalMaterial(const PositionState& pos);
	void evalPieceSquare(const PositionState& pos);  
	void evalMobility(const PositionState& pos);

	//position value in centi pawns
	int16_t _posValue;

	// Material table of the material values
	// for usual cases of material (no extra promoted pieces)
	MaterialInfo* _materialTable;
};

}

#endif
