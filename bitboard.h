/*bitboard.h*/

#ifndef BITBOARD_H
#define BITBOARD_H

#include "bitnum64.h"

//This is the general state describing bitboard class

class Bitboard
{
	public:
		Bitboard(const Number& num);

		void set(const Number& num);
		Number get() const;
		const std::string to_string(); 

	private:
		Number num_;
};

#endif
