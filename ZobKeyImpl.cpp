#include "ZobKeyImpl.h"
#include <assert.h>
#include <cstdlib>

namespace pismo
{
ZobKeyImpl::ZobKeyImpl()
{
	std::srand(RANDOM_GENERATOR_SEED);
	initPieceAtSquareKeys();
	initBlackToPlayKey();
	initEnPassantKeys();
	initCastlingKeys();

	//for _materialZobKey
	initMaterialKeys();
}

ZobKey ZobKeyImpl::getPieceAtSquareKey(Piece piece, Square sq) const
{
	assert((piece != PAWN_WHITE && piece != PAWN_BLACK) || (sq >= A2 && sq <= H7));
	return _pieceAtSquareKeys[piece][sq];
}

ZobKey ZobKeyImpl::getIfBlackToPlayKey() const
{
	return _blackToPlayKey;
}

ZobKey ZobKeyImpl::getEnPassantKey(unsigned int enPassantFile) const
{
	assert(enPassantFile < 8);
	return _enPassantKeys[enPassantFile];
}

ZobKey ZobKeyImpl::getWhiteLeftCastlingKey() const
{
	return _whiteLeftCastlingKey;
}

ZobKey ZobKeyImpl::getWhiteRightCastlingKey() const
{
	return _whiteRightCastlingKey;
}

ZobKey ZobKeyImpl::getBlackLeftCastlingKey() const
{
	return _blackLeftCastlingKey;
}

ZobKey ZobKeyImpl::getBlackRightCastlingKey() const
{
	return _blackRightCastlingKey;
}

ZobKey ZobKeyImpl::getMaterialKey(Piece piece, unsigned int count) const
{
      return _materialKeys[piece][count];
}

void ZobKeyImpl::initPieceAtSquareKeys()
{
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_COUNT; ++piece) {
		for (unsigned int square = A1; square < SQUARES_COUNT; ++square) {
			if ((piece != PAWN_WHITE && piece != PAWN_BLACK) || (square >= A2 && square <= H7)) {
				_pieceAtSquareKeys[piece][square] = getRandomNumber();
			}
		}
	}
}

void ZobKeyImpl::initBlackToPlayKey()
{
	_blackToPlayKey = getRandomNumber();
}

void ZobKeyImpl::initEnPassantKeys()
{
	for (unsigned int file = 0; file < 8; ++file) {
		_enPassantKeys[file] = getRandomNumber();
	}
}

void ZobKeyImpl::initCastlingKeys()
{
	_whiteLeftCastlingKey = getRandomNumber();	
	_whiteRightCastlingKey = getRandomNumber();
	_blackLeftCastlingKey = getRandomNumber();
	_blackRightCastlingKey = getRandomNumber();
}

void ZobKeyImpl::initMaterialKeys()
{
	//for kings it doesn't make sense to generate all values, but we did it to reduce code size
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_COUNT; ++piece) {
		for (unsigned int count = 0; count <= POSSIBLE_SAME_PIECES; ++count) {
			_materialKeys[piece][count] = getRandomNumber();
		}
	}
}


ZobKey ZobKeyImpl::getRandomNumber() const
{
	//TODO: Implement our own generator in future
	ZobKey tmp = std::rand();
	return ((tmp << 32) | std::rand());
}

}
