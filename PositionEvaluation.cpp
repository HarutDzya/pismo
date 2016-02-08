#include "PositionEvaluation.h"
#include "PositionState.h"
#include "BitboardImpl.h"

#include <assert.h>


//#define LOG_EVAL

#ifndef LOG_EVAL

#define incr(a, b, c) a+=b
#define decr(a, b, c) a-=b

#define incrScore(a, b, c, d) a._mgScore +=b; a._egScore += c;
#define decrScore(a, b, c, d) a._mgScore -=b; a._egScore -= c;

#define resetEvalLog()
#define printEvalLog(a)

#else
#include "LogEval.h"
#endif

namespace pismo
{

extern int bitCount(uint64_t);

uint16_t piecePhaseValue[PEACE_TYPE_COUNT] = {10, 32, 32, 50, 92, 0}; //all together
uint16_t maxPhase = 750; //a bit smaller than all pieces values
uint16_t minPhase = 50;  //a bit bigger than zero

//first index is stage of game (middlegame, endgame)
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

//bonuses and penalties for material imbalance
const int16_t KnightRedundancyPenalty = 10;
const int16_t KnightPawnsBonus = 6;
const int16_t KnightWithQueenBonus = 20;
const int16_t KnightOnlyPiecePenalty = 40;
const int16_t KnightWithFewPiecesPenalty = 4;
const int16_t BishopVSRookBonus = 20;
const int16_t BishopWithRookBonus = 10;
const int16_t BishopOnlyPiecePenalty = 35;
const int16_t BishopVSPawnsEndgamdBonus = 5;
const int16_t BishopWithFewPiecesPenalty = 3;
const int16_t BishopPairBonus = 50;
const int16_t BishopPairVSnoMinor = 15;
const int16_t BishopPairAndPawnsBonus = 2;
const int16_t RookPawnsBonus = 12;
const int16_t MajorPieceRedundancyPenalty = 30;
const int16_t KnightRookVSQueenBonus = 10;
const int16_t ExtraRookMinorVSQueenBonus = 10;
const int16_t QueenNoMinorPenalty = 30;
const int16_t QueenRedundancyPenalty = 75;


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
	_score(0, 0),
	_materialTable(0),
	_pawnHash(0),
	_currentPawnEval(0),
	_unusualMaterialPhase(0)
{
}

void PositionEvaluation::initPosEval()
{
	initMaterialTable();
	initPawnHash();
}

template <Color clr>
int16_t PositionEvaluation::materialImbalance(unsigned int pieceCount[])
{
  int16_t value = 0;
  Piece myPawn = PAWN_WHITE;
  Piece myKnight = KNIGHT_WHITE;
  Piece myBishop = BISHOP_WHITE;
  Piece myRook = ROOK_WHITE;
  Piece myQueen = QUEEN_WHITE;

  Piece oppPawn = PAWN_BLACK;
  Piece oppKnight = KNIGHT_BLACK;
  Piece oppBishop = BISHOP_BLACK;
  Piece oppRook = ROOK_BLACK;
  Piece oppQueen = QUEEN_BLACK;
  if (clr == BLACK)
  {
    myPawn = PAWN_BLACK;
    myKnight = KNIGHT_BLACK;
    myBishop = BISHOP_BLACK;
    myRook = ROOK_BLACK;
    myQueen = QUEEN_BLACK;

    oppPawn = PAWN_WHITE;
    oppKnight = KNIGHT_WHITE;
    oppBishop = BISHOP_WHITE;
    oppRook = ROOK_WHITE;
    oppQueen = QUEEN_WHITE;
  }

  int16_t maxPiecePhase = 2 * piecePhaseValue[KNIGHT] + 2 * piecePhaseValue[BISHOP] +
                          2 * piecePhaseValue[ROOK] + 1 * piecePhaseValue[QUEEN];

  int16_t currentPhase = pieceCount[myKnight] * piecePhaseValue[KNIGHT] + pieceCount[myBishop] * piecePhaseValue[BISHOP] +
                         pieceCount[myRook] * piecePhaseValue[ROOK] + pieceCount[myQueen] * piecePhaseValue[QUEEN];

  value = pieceCount[myPawn] * PIECE_VALUES[myPawn] +
      pieceCount[myKnight] * PIECE_VALUES[myKnight] +
      pieceCount[myBishop] * PIECE_VALUES[myBishop] +
      pieceCount[myRook] * PIECE_VALUES[myRook] +
      pieceCount[myQueen] * PIECE_VALUES[myQueen];

  //formalization of GM Larry Kaufman analysis on material imbalance
  //https://www.chess.com/blog/Jose_Rodriguez/evaluation-of-material-imbalances

  //KNIGHTS
  if (pieceCount[myKnight] > 0)
  {
    //penalty for two knights
    if (pieceCount[myKnight] == 2)
    {
      value -= KnightRedundancyPenalty;
    }

    // Raise the knight's value by 1/16 of pawn value for each pawn > 5 and lower by 1/16 for each pawn < 5
    value += pieceCount[myKnight] * (pieceCount[myPawn] - 5 ) * KnightPawnsBonus;

    //queen plus knight are better than queen plus bishop
    if (pieceCount[myQueen] == 1 && pieceCount[oppQueen] == 1 &&
        pieceCount[myRook] == pieceCount[oppRook] &&
        pieceCount[myKnight] == 1 && pieceCount[oppKnight] == 0 &&
        pieceCount[myBishop]  == 0 && pieceCount[oppBishop] == 1)
    {
      value += KnightWithQueenBonus;
    }

    //Penalty for the endgame with only piece is a knight
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 0 &&
       pieceCount[myRook] == 0 && pieceCount[oppRook] == 0 &&
       pieceCount[myBishop] == 0 && pieceCount[oppBishop] == 0 &&
       pieceCount[myKnight] == 1 && pieceCount[oppKnight] == 0)
    {
      value -= KnightOnlyPiecePenalty;
    }

    //The fewer pieces on the board, the fewer pawns a minor piece is worth
    value -=  (10 - currentPhase * 10 / maxPiecePhase) * KnightWithFewPiecesPenalty;

  }

  //BISHOPS
  if (pieceCount[myBishop] > 0)
  {
    // bishop is better than the knight against a rook
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 0 &&
       pieceCount[oppRook] - pieceCount[myRook] == 1 &&
       pieceCount[myBishop] == 1 && pieceCount[oppBishop] == 0 &&
       pieceCount[myKnight] == pieceCount[oppKnight])
    {
      value += BishopVSRookBonus;
    }

    // rook plus bishop are better than rook plus knight
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 0 &&
       pieceCount[myRook] == pieceCount[myRook] && pieceCount[myRook] > 0 &&
       pieceCount[myBishop] == 1 && pieceCount[oppBishop] == 0 &&
       pieceCount[myKnight] == 0 && pieceCount[oppKnight] == 1)
    {
      value += BishopWithRookBonus;
    }

    //Penalty for the endgame with only piece is a bishop
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 0 &&
       pieceCount[myRook] == 0 && pieceCount[oppRook] == 0 &&
       pieceCount[myBishop] == 1 && pieceCount[oppBishop] == 0 &&
       pieceCount[myKnight] == 0 && pieceCount[oppKnight] == 0)
    {
      value -= BishopOnlyPiecePenalty;
    }

    //Bishop is a bit better than the knight in the endgame against multiple pawns
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 0 &&
       pieceCount[myRook] == pieceCount[oppRook]  &&
       pieceCount[myBishop] == 1 && pieceCount[oppBishop] == 0 &&
       pieceCount[myKnight] == 0 && pieceCount[oppKnight] == 1)
    {
      if (8 - pieceCount[myPawn] > 5)
      {
        value += (pieceCount[myPawn] - 5) * BishopVSPawnsEndgamdBonus;
      }
    }

    //The fewer pieces on the board, the fewer pawns a minor piece is worth
    value -=  (10 - currentPhase * 10 / maxPiecePhase) * BishopWithFewPiecesPenalty;
  }

  //BISHOP PAIR
  if (pieceCount[myBishop] == 2)
  {
    value += BishopPairBonus;

    //bonus if opponent has no minor piece
    if ( pieceCount[oppKnight] == 0 && pieceCount[oppBishop] == 0)
    {
      value += BishopPairVSnoMinor;
    }

    // the bishop pair is worth less than half a pawn when most or all the pawns are on the board,
    // and more than half a pawn when half or more of the pawns are gone.
    value += (8 - (pieceCount[myPawn] + pieceCount[oppPawn])) * BishopPairAndPawnsBonus;
  }

  //ROOK
  if (pieceCount[myRook] > 0)
  {
    // Lower the rook's value by 1/8 of pawn value for each pawn > 5 and raise by 1/16 for each pawn < 5
    value += pieceCount[myRook] * (5 - pieceCount[myPawn]) * RookPawnsBonus;

    // penalty for major piece redundancy
    if (pieceCount[myRook] == 2 || pieceCount[myQueen] == 1)
    {
      value -= MajorPieceRedundancyPenalty;
    }
  }

  //QUEEN
  if (pieceCount[myQueen] == 1)
  {
    //Queen VS Rook + Minor
    if (pieceCount[myQueen] == 0 && pieceCount[oppQueen] == 1 &&
        pieceCount[myRook] - pieceCount[oppRook] == 1)
    {
      //The knight is marginally better than the bishop in assisting the rook against the queen
      if (pieceCount[myKnight] == 1 && pieceCount[oppKnight] == 0 &&
          pieceCount[myBishop] == pieceCount[oppBishop])
      {
        value += KnightRookVSQueenBonus;
      }

      //extra rooks are a bit better for the side having rook + minor
      if (pieceCount[myRook] == 2 &&
          (pieceCount[myBishop] - pieceCount[oppBishop] == 1 || pieceCount[myKnight] - pieceCount[oppKnight] == 1))
      {
        value += ExtraRookMinorVSQueenBonus;
      }
    }

    //Queen VS Rook pair
    if (pieceCount[myQueen] == 1 && pieceCount[oppQueen] == 0 &&
        pieceCount[myRook] - pieceCount[oppRook] == 2)
    {
      //penalty if there is no minor piece with queen
      if (pieceCount[myKnight] == 0 && pieceCount[myBishop] == 0)
      {
        value -= QueenNoMinorPenalty;
      }

      //penalty if there is only one minor with queen
      if (pieceCount[myKnight] + pieceCount[myBishop] == 1)
      {
        value -= QueenNoMinorPenalty / 2;
      }
    }
  }

  return value;
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

		_materialTable[index].value = materialImbalance<WHITE>(pieceCount) - materialImbalance<BLACK>(pieceCount);

		uint16_t p = piecePhaseValue[PAWN] * (pieceCount[PAWN_WHITE] + pieceCount[PAWN_BLACK]) +
		                              piecePhaseValue[KNIGHT] * (pieceCount[KNIGHT_WHITE] + pieceCount[KNIGHT_BLACK]) +
		                              piecePhaseValue[BISHOP] * (pieceCount[BISHOP_WHITE] + pieceCount[BISHOP_BLACK]) +
		                              piecePhaseValue[ROOK] * (pieceCount[ROOK_WHITE] + pieceCount[ROOK_BLACK]) +
		                              piecePhaseValue[QUEEN] * (pieceCount[QUEEN_WHITE] + pieceCount[QUEEN_BLACK]);


		//https://chessprogramming.wikispaces.com/Tapered+Eval

    if (p > maxPhase) p = maxPhase;
    else if (p < minPhase) p = minPhase;
    //linear mapping from range [minPhase, maxPhase] to [0, 128]
		_materialTable[index].phase = (p - minPhase) * 128 / (maxPhase - minPhase);
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
  _score._mgScore = 0;
  _score._egScore = 0;
	_currentPawnEval = 0;
  _freeSpace[WHITE] = 0;
  _freeSpace[BLACK] = 0;

	resetEvalLog();
}

/////////// evaluation

int16_t PositionEvaluation::evaluate(const PositionState& pos)
{
	reset(pos);

	incrScore(_score, _pos->getPstValue()._mgScore, _pos->getPstValue()._egScore, pst);

	int16_t mValue = evalMaterial();

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

	//interpolate value between MG and EG phases and add material
	// ( in material score phase should already be considered)
	int16_t phase = _pos->unusualMaterial() ? _unusualMaterialPhase : _materialTable[_pos->materialKey()].phase;
  printEvalLog(phase);

	int16_t value = (_score._mgScore * phase  + _score._egScore * (128 - (int)phase) ) / 128 + mValue;

	return value;
}

int16_t PositionEvaluation::evalMaterial()
{
	// TODO: Make the following improvement
	// http://www.talkchess.com/forum/viewtopic.php?topic_view=threads&p=340115&t=33561
	if (!_pos->unusualMaterial()) {
	  return _materialTable[_pos->materialKey()].value;
	}
	else {
    uint16_t p = piecePhaseValue[PAWN] * (_pos->_pieceCount[PAWN_WHITE] + _pos->_pieceCount[PAWN_BLACK]) +
                                  piecePhaseValue[KNIGHT] * (_pos->_pieceCount[KNIGHT_WHITE] + _pos->_pieceCount[KNIGHT_BLACK]) +
                                  piecePhaseValue[BISHOP] * (_pos->_pieceCount[BISHOP_WHITE] + _pos->_pieceCount[BISHOP_BLACK]) +
                                  piecePhaseValue[ROOK] * (_pos->_pieceCount[ROOK_WHITE] + _pos->_pieceCount[ROOK_BLACK]) +
                                  piecePhaseValue[QUEEN] * (_pos->_pieceCount[QUEEN_WHITE] + _pos->_pieceCount[QUEEN_BLACK]);


    if (p > maxPhase) p = maxPhase;
    else if (p < minPhase) p = minPhase;
    //linear mapping from range [minPhase, maxPhase] to [0, 128]
    _unusualMaterialPhase = (p - minPhase) * 128 / (maxPhase - minPhase);

    int16_t value =  _pos->_pieceCount[PAWN_WHITE] * PIECE_VALUES[PAWN_WHITE] +
        _pos->_pieceCount[KNIGHT_WHITE] * PIECE_VALUES[KNIGHT_WHITE] +
        _pos->_pieceCount[BISHOP_WHITE] * PIECE_VALUES[BISHOP_WHITE] +
        _pos->_pieceCount[ROOK_WHITE] * PIECE_VALUES[ROOK_WHITE] +
        _pos->_pieceCount[QUEEN_WHITE] * PIECE_VALUES[QUEEN_WHITE] +
        _pos->_pieceCount[KING_WHITE] * PIECE_VALUES[KING_WHITE] -
        _pos->_pieceCount[PAWN_BLACK] * PIECE_VALUES[PAWN_BLACK] -
        _pos->_pieceCount[KNIGHT_BLACK] * PIECE_VALUES[KNIGHT_BLACK] -
        _pos->_pieceCount[BISHOP_BLACK] * PIECE_VALUES[BISHOP_BLACK] -
        _pos->_pieceCount[ROOK_BLACK] * PIECE_VALUES[ROOK_BLACK] -
        _pos->_pieceCount[QUEEN_BLACK] * PIECE_VALUES[QUEEN_BLACK] -
        _pos->_pieceCount[KING_BLACK] * PIECE_VALUES[KING_BLACK];

    //penalty for two Queens
    value -= QueenRedundancyPenalty;
    return value;
	}
}

void PositionEvaluation::evalPawnsState()
{
	_currentPawnEval = &_pawnHash[_pos->getPawnKey() & PAWN_HASH_INDEX_MASK];
	if (_currentPawnEval->key == _pos->getPawnKey()) {
		_freeSpace[WHITE] = ~(_currentPawnEval->blackPawnAttacks |
	                    _pos->getPiecePos()[PAWN_WHITE] | _pos->getPiecePos()[KING_WHITE]);
	
		_freeSpace[BLACK] = ~(_currentPawnEval->whitePawnAttacks |
	                  _pos->getPiecePos()[PAWN_BLACK] | _pos->getPiecePos()[KING_BLACK]);
	    return;
	}
	
	_currentPawnEval->key = _pos->getPawnKey();
	
	_currentPawnEval->whitePawnAttacks = BitboardImpl::instance()->whitePawnAnyAttacks(_pos->getPiecePos()[PAWN_WHITE]);
	_currentPawnEval->blackPawnAttacks = BitboardImpl::instance()->blackPawnAnyAttacks(_pos->getPiecePos()[PAWN_BLACK]);
	
	_freeSpace[WHITE] = ~(_currentPawnEval->blackPawnAttacks |
						_pos->getPiecePos()[PAWN_WHITE] | _pos->getPiecePos()[KING_WHITE]);

	_freeSpace[BLACK] = ~(_currentPawnEval->whitePawnAttacks |
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
		count = bitCount(BitboardImpl::instance()->knightAttackFrom(from) & _freeSpace[clr]);
		if (clr == WHITE) {
			incrScore(_score, KnightMobility[MIDDLE_GAME][count], KnightMobility[END_GAME][count], whiteMobility);
		}
		else {
			decrScore(_score, KnightMobility[MIDDLE_GAME][count], KnightMobility[END_GAME][count], blackMobility);
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
		count = bitCount(BitboardImpl::instance()->bishopAttackFrom(from, _pos->occupiedSquares()) & _freeSpace[clr]);
		if (clr == WHITE) {
			incrScore(_score, BishopMobility[MIDDLE_GAME][count], BishopMobility[END_GAME][count], whiteMobility);
		} else {
			decrScore(_score, BishopMobility[MIDDLE_GAME][count], BishopMobility[END_GAME][count], blackMobility);
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
    count = bitCount(BitboardImpl::instance()->rookAttackFrom(from, _pos->occupiedSquares()) & _freeSpace[clr]);
		if (clr == WHITE) {
			incrScore(_score, RookMobility[MIDDLE_GAME][count], RookMobility[END_GAME][count], whiteMobility);
		} else {
			decrScore(_score, RookMobility[MIDDLE_GAME][count], RookMobility[END_GAME][count], blackMobility);
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
    count = bitCount(BitboardImpl::instance()->queenAttackFrom(from, _pos->occupiedSquares()) & _freeSpace[clr]);
		if (clr == WHITE) {
			incrScore(_score, QueenMobility[MIDDLE_GAME][count], QueenMobility[END_GAME][count], whiteMobility);
		} else {
			decrScore(_score, QueenMobility[MIDDLE_GAME][count], QueenMobility[END_GAME][count], blackMobility);
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







