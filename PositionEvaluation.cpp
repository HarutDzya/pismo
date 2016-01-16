#include "PositionEvaluation.h"
#include "PositionState.h"

namespace pismo
{

const uint16_t pieceIndexForMaterialTable[PIECE_NB] =
{
	2 * 2 * 3 * 3 * 3 * 3 * 3 * 3,
	2 * 2, 2 * 2 * 3 * 3,
	2 * 2 * 3 * 3 * 3 * 3,
	1, 0,
	2 * 2 * 3 * 3 * 3 * 3 * 3 * 3 * 9,
	2 * 2 * 3, 2 * 2 * 3 * 3 * 3,
	2 * 2 * 3 * 3 * 3 * 3 * 3,
	2, 0
};

const uint8_t initialNumberOfPieces[PIECE_NB] =
{
	8, 2, 2, 2, 1, 1,
	8, 2, 2, 2, 1, 1
};

const uint16_t pieceMask[PIECE_NB] =
{
	1 << 0, 1 << 1, 1 << 2, 1 << 3,
	1 << 4, 1 << 5, 1 << 6, 1 << 7,
	1 << 8, 1 << 9, 1 << 10, 1 << 11
};


PositionEvaluation::PositionEvaluation():
	_posValue(0),
	_materialTable(0),
	_pawnHash(0)
{
}

void PositionEvaluation::initPosEval()
{
	initMaterialTable();
	initPawnHash();
}

void PositionEvaluation::initMaterialTable()
{
	_materialTable = new MaterialInfo[MATERIAL_TABLE_SIZE];
	unsigned int pieceCount[PIECE_NB];
	for (unsigned int index = 0; index < MATERIAL_TABLE_SIZE; ++index) {
		// Order of finding pieceCount here depends on the value
		// of pieceIndexForMaterialTable (big to small)
		unsigned int tempIndex = index;
		pieceCount[PAWN_BLACK] = tempIndex / pieceIndexForMaterialTable[PAWN_BLACK];
		tempIndex %= pieceIndexForMaterialTable[PAWN_BLACK];
		pieceCount[PAWN_WHITE] = tempIndex / pieceIndexForMaterialTable[PAWN_WHITE];
		tempIndex %= pieceIndexForMaterialTable[PAWN_WHITE];
		pieceCount[ROOK_BLACK] = tempIndex / pieceIndexForMaterialTable[ROOK_BLACK];
		tempIndex %= pieceIndexForMaterialTable[ROOK_BLACK];
		pieceCount[ROOK_WHITE] = tempIndex / pieceIndexForMaterialTable[ROOK_WHITE];
		tempIndex %= pieceIndexForMaterialTable[ROOK_WHITE];
		pieceCount[BISHOP_BLACK] = tempIndex / pieceIndexForMaterialTable[BISHOP_BLACK];
		tempIndex %= pieceIndexForMaterialTable[BISHOP_BLACK];
		pieceCount[BISHOP_WHITE] = tempIndex / pieceIndexForMaterialTable[BISHOP_WHITE];
		tempIndex %= pieceIndexForMaterialTable[BISHOP_WHITE];
		pieceCount[KNIGHT_BLACK] = tempIndex / pieceIndexForMaterialTable[KNIGHT_BLACK];
		tempIndex %= pieceIndexForMaterialTable[KNIGHT_BLACK];
		pieceCount[KNIGHT_WHITE] = tempIndex / pieceIndexForMaterialTable[KNIGHT_WHITE];
		tempIndex %= pieceIndexForMaterialTable[KNIGHT_WHITE];
		pieceCount[QUEEN_BLACK] = tempIndex / pieceIndexForMaterialTable[QUEEN_BLACK];
		tempIndex %= pieceIndexForMaterialTable[QUEEN_BLACK];
		pieceCount[QUEEN_WHITE] = tempIndex / pieceIndexForMaterialTable[QUEEN_WHITE];
		tempIndex %= pieceIndexForMaterialTable[QUEEN_WHITE];
		pieceCount[KING_BLACK] = 1;
		pieceCount[KING_WHITE] = 1;
		_materialTable[index].value = pieceCount[PAWN_WHITE] * PIECE_VALUES[PAWN_WHITE] +
			pieceCount[KNIGHT_WHITE] * PIECE_VALUES[KNIGHT_WHITE] +
			pieceCount[BISHOP_WHITE] * PIECE_VALUES[BISHOP_WHITE] +
			pieceCount[ROOK_WHITE] * PIECE_VALUES[ROOK_WHITE] +
			pieceCount[QUEEN_WHITE] * PIECE_VALUES[QUEEN_WHITE] +
			pieceCount[KING_WHITE] * PIECE_VALUES[KING_WHITE] -
			pieceCount[PAWN_BLACK] * PIECE_VALUES[PAWN_BLACK] -
			pieceCount[KNIGHT_BLACK] * PIECE_VALUES[KNIGHT_BLACK] -
			pieceCount[BISHOP_BLACK] * PIECE_VALUES[BISHOP_BLACK] - 
			pieceCount[ROOK_BLACK] * PIECE_VALUES[ROOK_BLACK] -
			pieceCount[QUEEN_BLACK] * PIECE_VALUES[QUEEN_BLACK] -
			pieceCount[KING_BLACK] * PIECE_VALUES[KING_BLACK];
	}
}

void PositionEvaluation::initPawnHash()
{
	_pawnHash = new PawnEvalInfo[PAWN_HASH_SIZE];
	for (unsigned int index = 0; index < PAWN_HASH_SIZE; ++index) {
		_pawnHash[index].score = 0;
		_pawnHash[index].key = 0;
	}
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
	// TODO: Make the following improvement
	// http://www.talkchess.com/forum/viewtopic.php?topic_view=threads&p=340115&t=33561
	if (!pos.unusualMaterial()) {
		_posValue += _materialTable[pos.materialKey()].value;
	}
	else {
		_posValue += pos.getPieceCount()[PAWN_WHITE] * PIECE_VALUES[PAWN_WHITE] +
			pos.getPieceCount()[KNIGHT_WHITE] * PIECE_VALUES[KNIGHT_WHITE] +
			pos.getPieceCount()[BISHOP_WHITE] * PIECE_VALUES[BISHOP_WHITE] +
			pos.getPieceCount()[ROOK_WHITE] * PIECE_VALUES[ROOK_WHITE] +
			pos.getPieceCount()[QUEEN_WHITE] * PIECE_VALUES[QUEEN_WHITE] +
			pos.getPieceCount()[KING_WHITE] * PIECE_VALUES[KING_WHITE] -
			pos.getPieceCount()[PAWN_BLACK] * PIECE_VALUES[PAWN_BLACK] -
			pos.getPieceCount()[KNIGHT_BLACK] * PIECE_VALUES[KNIGHT_BLACK] -
			pos.getPieceCount()[BISHOP_BLACK] * PIECE_VALUES[BISHOP_BLACK] - 
			pos.getPieceCount()[ROOK_BLACK] * PIECE_VALUES[ROOK_BLACK] -
			pos.getPieceCount()[QUEEN_BLACK] * PIECE_VALUES[QUEEN_BLACK] -
			pos.getPieceCount()[KING_BLACK] * PIECE_VALUES[KING_BLACK];
	}
}

void PositionEvaluation::evalPawnsState(const PositionState& pos)
{
	PawnEvalInfo* pawnEval = &_pawnHash[pos.getPawnKey() & PAWN_HASH_INDEX_MASK];
	if (pawnEval->key != pos.getPawnKey()) {
		pawnEval->key = pos.getPawnKey();
		pawnEval->score = evaluatePawns(pos);
	}

	_posValue += pawnEval->score;
}

int16_t PositionEvaluation::evaluatePawns(const PositionState& pos) const
{
	// TODO: Add pawn state evaluation
	return 0;
}

void PositionEvaluation::evalPieceSquare(const PositionState& pos)
{
	_posValue += pos.getPstValue();
}

void PositionEvaluation::evalMobility(const PositionState& pos)
{
}

PositionEvaluation::~PositionEvaluation()
{
	delete[] _materialTable;
	delete[] _pawnHash;
}

}







