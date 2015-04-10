#include <iostream>
#include "bitboard.h"


int main()
{
	Bitboard testboard=Number(0xFFFF00000000FFFF);
	std::cout<<testboard.to_string();
}
