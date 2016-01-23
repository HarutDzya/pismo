#include "PositionEvaluation.h"
#include "PositionState.h"
#include "BitboardImpl.h"

#include <assert.h>

namespace pismo
{

extern int bitCount(uint64_t);

//first index is stage of game (begin, end)
//second index is number of attacked squares

int KnightMobility[2][9] =
{
	{ 0, 5, 15, 25, 30, 33, 36, 38, 40},
	{ 0, 5, 15, 25, 30, 33, 36, 38, 40}
};

int BishopMobility[2][14] =
{
	{ 0, 5, 15, 25, 30, 33, 36, 39, 41, 43, 45, 47, 49, 50 },
	{ 0, 5, 15, 25, 30, 33, 36, 39, 41, 43, 45, 47, 49, 50 }
};

int RookMobility[2][16] =
{
	{ 0, 3, 10, 20, 30, 33, 36, 38, 40, 42, 44, 46, 47, 48, 49, 50 },
	{ 0, 3, 10, 20, 30, 33, 36, 38, 40, 42, 44, 46, 47, 48, 49, 50 },
};


int QueenMobility[2][28] =
{
	{
		0, 3, 7, 15, 25, 27, 28, 30, 32, 34, 36, 38, 40, 42, 43,
		44, 45, 46, 47, 47, 47, 47, 48, 48, 48, 49, 49, 50
	},
	{
	  0, 3, 7, 15, 25, 27, 28, 30, 32, 34, 36, 38, 40, 42, 43,
	  44, 45, 46, 47, 47, 47, 47, 48, 48, 48, 49, 49, 50
	}
};


const uint16_t pieceIndexForMaterialTable[PIECE_COUNT] =
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

const uint8_t initialNumberOfPieces[PIECE_COUNT] =
{
	8, 2, 2, 2, 1, 1,
	8, 2, 2, 2, 1, 1
};

const uint16_t pieceMask[PIECE_COUNT] =
{
	1 << 0, 1 << 1, 1 << 2, 1 << 3,
	1 << 4, 1 << 5, 1 << 6, 1 << 7,
	1 << 8, 1 << 9, 1 << 10, 1 << 11
};


PositionEvaluation::PositionEvaluation():
	_value(0),
	_materialTable(0),
	_pawnHash(0),
	_currentPawnEval(0),
	_whiteFreeSpace(0),
	_blackFreeSpace(0)
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
	unsigned int pieceCount[PIECE_COUNT];
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

void PositionEvaluation::reset(const PositionState& pos)
{
	//TODO: Init PositionState globally, and do not pass it into functions
	_pos = &pos;

	//TODO: memcpy is somewhat faster here
	_currentPawnEval = 0;
	_whiteFreeSpace = 0;
	_blackFreeSpace = 0;
}

/////////// evaluation

int16_t PositionEvaluation::evaluate(const PositionState& pos)
{
	reset(pos);

	_value = _pos->getPstValue();

	evalMaterial();

	evalPawnsState();

	evalKnights<WHITE>();
	evalKnights<BLACK>();

	evalBishops<WHITE>();
	evalBishops<BLACK>();

	evalRooks<WHITE>();
	evalRooks<BLACK>();

	evalQueens<WHITE>();
	evalQueens<BLACK>();

	// evalKingSafety();
	
	return _value;
}

void PositionEvaluation::evalMaterial()
{
	// TODO: Make the following improvement
	// http://www.talkchess.com/forum/viewtopic.php?topic_view=threads&p=340115&t=33561
	if (!_pos->unusualMaterial()) {
	  _value += _materialTable[_pos->materialKey()].value;
	}
	else {
	  _value += _pos->getPieceCount()[PAWN_WHITE] * PIECE_VALUES[PAWN_WHITE] +
	      _pos->getPieceCount()[KNIGHT_WHITE] * PIECE_VALUES[KNIGHT_WHITE] +
	      _pos->getPieceCount()[BISHOP_WHITE] * PIECE_VALUES[BISHOP_WHITE] +
	      _pos->getPieceCount()[ROOK_WHITE] * PIECE_VALUES[ROOK_WHITE] +
	      _pos->getPieceCount()[QUEEN_WHITE] * PIECE_VALUES[QUEEN_WHITE] +
	      _pos->getPieceCount()[KING_WHITE] * PIECE_VALUES[KING_WHITE] -
	      _pos->getPieceCount()[PAWN_BLACK] * PIECE_VALUES[PAWN_BLACK] -
	      _pos->getPieceCount()[KNIGHT_BLACK] * PIECE_VALUES[KNIGHT_BLACK] -
	      _pos->getPieceCount()[BISHOP_BLACK] * PIECE_VALUES[BISHOP_BLACK] -
	      _pos->getPieceCount()[ROOK_BLACK] * PIECE_VALUES[ROOK_BLACK] -
	      _pos->getPieceCount()[QUEEN_BLACK] * PIECE_VALUES[QUEEN_BLACK] -
	      _pos->getPieceCount()[KING_BLACK] * PIECE_VALUES[KING_BLACK];
	}
}

void PositionEvaluation::evalPawnsState()
{
	_currentPawnEval = &_pawnHash[_pos->getPawnKey() & PAWN_HASH_INDEX_MASK];
	if (_currentPawnEval->key == _pos->getPawnKey()) {
		_whiteFreeSpace = ~(_currentPawnEval->blackPawnAttacks |
	                    _pos->getPiecePos()[PAWN_WHITE] | _pos->getPiecePos()[KING_WHITE]);
	
		_blackFreeSpace = ~(_currentPawnEval->whitePawnAttacks |
	                  _pos->getPiecePos()[PAWN_BLACK] | _pos->getPiecePos()[KING_BLACK]);
	    return;
	}
	
	_currentPawnEval->key = _pos->getPawnKey();
	
	_currentPawnEval->whitePawnAttacks = BitboardImpl::instance()->whitePawnAnyAttacks(_pos->getPiecePos()[PAWN_WHITE]);
	_currentPawnEval->blackPawnAttacks = BitboardImpl::instance()->blackPawnAnyAttacks(_pos->getPiecePos()[PAWN_BLACK]);
	
	_whiteFreeSpace = ~(_currentPawnEval->blackPawnAttacks |
						_pos->getPiecePos()[PAWN_WHITE] | _pos->getPiecePos()[KING_WHITE]);

	_blackFreeSpace = ~(_currentPawnEval->whitePawnAttacks |
						_pos->getPiecePos()[PAWN_BLACK] | _pos->getPiecePos()[KING_BLACK]);
}

template <Color clr>
void PositionEvaluation::evalKnights()
{
	assert(_currentPawnEval);
	
	Bitboard knightsPos = _pos->getPiecePos()[clr == WHITE ? KNIGHT_WHITE : KNIGHT_BLACK];
	int count = 0;
	Square from = INVALID_SQUARE;
	
	while (knightsPos) {
		from = (Square)BitboardImpl::instance()->lsb(knightsPos);
		if (clr == WHITE) {
			count = bitCount(BitboardImpl::instance()->knightAttackFrom(from) & _whiteFreeSpace);
			_value = _value + KnightMobility[BEGINNING][count];
		}
		else {
			count = bitCount(BitboardImpl::instance()->knightAttackFrom(from) & _blackFreeSpace);
			_value = _value - KnightMobility[BEGINNING][count];
		}

		knightsPos &= (knightsPos - 1);
	}
}

template <Color clr>
void PositionEvaluation::evalBishops()
{
	Bitboard bishopsPos = _pos->getPiecePos()[clr == WHITE ? BISHOP_WHITE : BISHOP_BLACK];
	int count = 0;
	Square from = INVALID_SQUARE;
	
	while (bishopsPos) {
		from = (Square)BitboardImpl::instance()->lsb(bishopsPos);
		if (clr == WHITE) {
			count = bitCount(BitboardImpl::instance()->bishopAttackFrom(from, _pos->occupiedSquares()) & _whiteFreeSpace);
			_value = _value + BishopMobility[BEGINNING][count];
		} else {
			count = bitCount(BitboardImpl::instance()->bishopAttackFrom(from, _pos->occupiedSquares()) & _blackFreeSpace);
			_value = _value - BishopMobility[BEGINNING][count];
		}

		bishopsPos &= (bishopsPos - 1);
	}
}

template <Color clr>
void PositionEvaluation::evalRooks()
{
	Bitboard rooksPos = _pos->getPiecePos()[clr == WHITE ? ROOK_WHITE : ROOK_BLACK];
	int count = 0;
	Square from = INVALID_SQUARE;

	while (rooksPos) {
		from = (Square)BitboardImpl::instance()->lsb(rooksPos);
		if (clr == WHITE) {
			count = bitCount(BitboardImpl::instance()->rookAttackFrom(from, _pos->occupiedSquares()) & _whiteFreeSpace);
			_value = _value + RookMobility[BEGINNING][count];
		} else {
			count = bitCount(BitboardImpl::instance()->rookAttackFrom(from, _pos->occupiedSquares()) & _blackFreeSpace);
			_value = _value - RookMobility[BEGINNING][count];
		}

		rooksPos &= (rooksPos - 1);
	}
}

template <Color clr>
void PositionEvaluation::evalQueens()
{
	Bitboard queensPos = _pos->getPiecePos()[clr == WHITE ? QUEEN_WHITE : QUEEN_BLACK];
	int count = 0;
	Square from = INVALID_SQUARE;

	while (queensPos) {
		from = (Square)BitboardImpl::instance()->lsb(queensPos);
		if (clr == WHITE) {
			count = bitCount(BitboardImpl::instance()->queenAttackFrom(from, _pos->occupiedSquares()) & _whiteFreeSpace);
			_value = _value + QueenMobility[BEGINNING][count];
		} else {
			count = bitCount(BitboardImpl::instance()->queenAttackFrom(from, _pos->occupiedSquares()) & _blackFreeSpace);
			_value = _value - QueenMobility[BEGINNING][count];
		}

		queensPos &= (queensPos - 1);
	}
}

PositionEvaluation::~PositionEvaluation()
{
	delete[] _materialTable;
	delete[] _pawnHash;
}

}







