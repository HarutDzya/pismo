#include "bitboard.h"

Bitboard::Bitboard(const Number& num): num_(num)
{
}

void Bitboard::set(const Number& num)
{
	num_=num;
}

Number Bitboard::get() const
{
	return num_;
}

const std::string Bitboard::to_string()
{
	return num_.to_string();
}
