#include "utils.h"
#include <string>

namespace pismo
{

Bitboard squareToBitboard[NUMBER_OF_SQUARES] =
{
	1L << 0,  1L << 1,  1L << 2,  1L << 3,  1L << 4,  1L << 5,  1L << 6,  1L << 7,
	1L << 8,  1L << 9,  1L << 10, 1L << 11, 1L << 12, 1L << 13, 1L << 14, 1L << 15,
	1L << 16, 1L << 17, 1L << 18, 1L << 19, 1L << 20, 1L << 21, 1L << 22, 1L << 23,
	1L << 24, 1L << 25, 1L << 26, 1L << 27, 1L << 28, 1L << 29, 1L << 30, 1L << 31,
	1L << 32, 1L << 33, 1L << 34, 1L << 35, 1L << 36, 1L << 37, 1L << 38, 1L << 39,
	1L << 40, 1L << 41, 1L << 42, 1L << 43, 1L << 44, 1L << 45, 1L << 46, 1L << 47,
	1L << 48, 1L << 49, 1L << 50, 1L << 51, 1L << 52, 1L << 53, 1L << 54, 1L << 55,
	1L << 56, 1L << 57, 1L << 58, 1L << 59, 1L << 60, 1L << 61, 1L << 62, 1L << 63
};
 

std::string getPromoted(Piece piece)
{
  switch (piece)
  {
    case BISHOP_WHITE:
    case BISHOP_BLACK:
      return "b";
    case KNIGHT_WHITE:
    case KNIGHT_BLACK:
      return "n";
    case ROOK_WHITE:
    case ROOK_BLACK:
      return "r";
    case QUEEN_WHITE:
    case QUEEN_BLACK:
      return "q";
    default:
      return "";
  }
  return "";
}

//we should implement png notation at some point
//for now just any printable format for debugging
std::string moveToNotation(const MoveInfo& move)
{
  std::string str;
	str += char(65 + move.from % 8); 
	str += char(49 + move.from / 8);
	str += "->";
	str += char(65 + move.to % 8); 
	str += char(49 + move.to / 8);
	str += getPromoted(move.promoted);

  return str;
}

}
