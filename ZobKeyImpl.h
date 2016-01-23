#ifndef ZOBKEYIMPL_H_
#define ZOBKEYIMPL_H_

#include "utils.h"

namespace pismo
{
const unsigned int RANDOM_GENERATOR_SEED = 10;
const unsigned int POSSIBLE_SAME_PIECES = 10; //e.g. if all pawns promoted to rooks there will be 10 rooks

class ZobKeyImpl
{
public:
	ZobKeyImpl();

	ZobKey getPieceAtSquareKey(Piece piece, Square sq) const;
	ZobKey getIfBlackToPlayKey() const;
	ZobKey getEnPassantKey(unsigned int enPassantFile) const;
	ZobKey getWhiteLeftCastlingKey() const;
	ZobKey getWhiteRightCastlingKey() const;
	ZobKey getBlackLeftCastlingKey() const;
	ZobKey getBlackRightCastlingKey() const;

	//material table
	ZobKey getMaterialKey(Piece piece, unsigned int count) const;

private:
	void initPieceAtSquareKeys();
	void initBlackToPlayKey();
	void initEnPassantKeys();
	void initCastlingKeys();

  void initMaterialKeys();

	ZobKey getRandomNumber() const;

	ZobKey _pieceAtSquareKeys[PIECE_COUNT][SQUARES_COUNT];
	ZobKey _blackToPlayKey;
	ZobKey _enPassantKeys[8];
	ZobKey _whiteLeftCastlingKey;
	ZobKey _whiteRightCastlingKey;
	ZobKey _blackLeftCastlingKey;
	ZobKey _blackRightCastlingKey;

	ZobKey _materialKeys[PIECE_COUNT][POSSIBLE_SAME_PIECES + 1];
};

}

#endif
