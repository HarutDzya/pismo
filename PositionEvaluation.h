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

extern const uint16_t pieceIndexForMaterialTable[PIECE_COUNT];
extern const uint8_t initialNumberOfPieces[PIECE_COUNT];
extern const uint16_t pieceMask[PIECE_COUNT];

struct PawnEvalInfo
{
	int16_t score;
	ZobKey key;
	Bitboard whitePawnAttacks;
	Bitboard blackPawnAttacks;
};

const unsigned int PAWN_HASH_SIZE = 1 << 15;
const unsigned int PAWN_HASH_INDEX_MASK = PAWN_HASH_SIZE - 1;


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

	void initPosEval();

	int16_t evaluate(const PositionState& pos);
  

private:
	void reset(const PositionState& pos);

	// Initializes material table to precomputed
	// values using materialPieceIndex to compute
	// index
	void initMaterialTable();

	// Evaluates position material and adds the
	// value to the _posValue
	void evalMaterial();


	// Initializes pawn hash table, by allocating space
	// and assigning all entries to 0 value
	void initPawnHash();

	//evaluate pawn structure
	void evalPawnsState();

	template <Color clr>
	void evalKnights();

	template <Color clr>
	void evalBishops();

	template <Color clr>
	void evalRooks();

	template <Color clr>
	void evalQueens();

	//Current position value in centi pawns
	int16_t _value;

	// Material table of the material values
	// for usual cases of material (no extra promoted pieces)
	MaterialInfo* _materialTable;

	// Hash table of the pawn state evaluation
	PawnEvalInfo* _pawnHash;

	PawnEvalInfo* _currentPawnEval;

	const PositionState* _pos;

	Bitboard _whiteFreeSpace;
	Bitboard _blackFreeSpace;
};

}

#endif
