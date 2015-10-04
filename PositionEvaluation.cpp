#include "PositionEvaluation.h"
#include "PositionState.h"

namespace pismo
{
PositionEvaluation::PositionEvaluation():
	_posValue(0),
	_materialHash(MATERIAL_HASH_SIZE)
{
}


int16_t PositionEvaluation::evaluate(const PositionState& pos)
{
	_posValue = 0;
	evalMaterial(pos);
	evalPieceSquare(pos);
	evalMobility(pos);
	// kingSafety();
	
	return _posValue;
}

void PositionEvaluation::evalMaterial(const PositionState& pos)
{
	int16_t materialValue = 0;
	unsigned int index = hashFunction(pos.getMaterialZobKey());
	if (_materialHash[index].materialZobKey == pos.getMaterialZobKey()) {
		materialValue = _materialHash[index].materialValue;
	}
	else {	
		for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
			materialValue += pos.getPieceCount((Piece) piece) * PIECE_VALUES[piece];
		}
		_materialHash[index] = MaterialInfo(materialValue, pos.getMaterialZobKey());
	}
	
	_posValue += materialValue;
}

void PositionEvaluation::evalPieceSquare(const PositionState& pos)
{
	_posValue += pos.getPstValue();
}

void PositionEvaluation::evalMobility(const PositionState& pos)
{
}

unsigned int PositionEvaluation::hashFunction(const ZobKey& materialZobKey) const
{
	return (materialZobKey % MATERIAL_HASH_SIZE);
       // TODO: Change the impl to return first log(MATERIAL_HASH_SIZE) + 1 bits of materialZobKey	

}

}







