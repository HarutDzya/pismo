#include "ZobKeyImpl.h"
#include <assert.h>
#include <cstdlib>

namespace pismo
{
ZobKeyImpl::ZobKeyImpl()
{
	std::srand(RANDOM_GENERATOR_SEED);
	init_piece_at_square_keys();
	init_black_to_play_key();
	init_en_passant_keys();
	init_castling_keys();
}

ZobKey ZobKeyImpl::get_piece_at_square_key(Piece piece, Square sq) const
{
	assert((piece != PAWN_WHITE && piece != PAWN_BLACK) || (sq >= A2 && sq <= H7));
	return _piece_at_square_keys[piece][sq];
}

ZobKey ZobKeyImpl::get_if_black_to_play_key() const
{
	return _black_to_play_key;
}

ZobKey ZobKeyImpl::get_en_passant_key(unsigned int en_passant_file) const
{
	assert(en_passant_file < 8);
	return _en_passant_keys[en_passant_file];
}

ZobKey ZobKeyImpl::get_white_left_castling_key() const
{
	return _white_left_castling_key;
}

ZobKey ZobKeyImpl::get_white_right_castling_key() const
{
	return _white_right_castling_key;
}

ZobKey ZobKeyImpl::get_black_left_castling_key() const
{
	return _black_left_castling_key;
}

ZobKey ZobKeyImpl::get_black_right_castling_key() const
{
	return _black_right_castling_key;
}

void ZobKeyImpl::init_piece_at_square_keys()
{
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
		for (unsigned int square = A1; square < NUMBER_OF_SQUARES; ++square) {
			if ((piece != PAWN_WHITE && piece != PAWN_BLACK) || (square >= A2 && square <= H7)) {
				_piece_at_square_keys[piece][square] = get_random_number();
			}
		}
	}
}

void ZobKeyImpl::init_black_to_play_key()
{
	_black_to_play_key = get_random_number();
}

void ZobKeyImpl::init_en_passant_keys()
{
	for (unsigned int file = 0; file < 8; ++file) {
		_en_passant_keys[file] = get_random_number();
	}
}

void ZobKeyImpl::init_castling_keys()
{
	_white_left_castling_key = get_random_number();	
	_white_right_castling_key = get_random_number();
	_black_left_castling_key = get_random_number();
	_black_right_castling_key = get_random_number();
}

ZobKey ZobKeyImpl::get_random_number() const
{
	ZobKey tmp = std::rand();
	return ((tmp << 32) | std::rand());
}
}
