#ifndef ZOBKEYIMPL_H_
#define ZOBKEYIMPL_H_

#include "utils.h"

namespace pismo
{
const unsigned int RANDOM_GENERATOR_SEED = 10;

class ZobKeyImpl
{
public:
	ZobKeyImpl();

	ZobKey get_piece_at_square_key(Piece piece, Square sq) const;
	ZobKey get_if_black_to_play_key() const;
	ZobKey get_en_passant_key(unsigned int en_passant_file) const;
	ZobKey get_white_left_castling_key() const;
	ZobKey get_white_right_castling_key() const;
	ZobKey get_black_left_castling_key() const;
	ZobKey get_black_right_castling_key() const;

private:
	void init_piece_at_square_keys();
	void init_black_to_play_key();
	void init_en_passant_keys();
	void init_castling_keys();
	ZobKey get_random_number() const;

	ZobKey _piece_at_square_keys[PIECE_NB][NUMBER_OF_SQUARES];
	ZobKey _black_to_play_key;
	ZobKey _en_passant_keys[8];
	ZobKey _white_left_castling_key;
	ZobKey _white_right_castling_key;
	ZobKey _black_left_castling_key;
	ZobKey _black_right_castling_key;
};

}

#endif