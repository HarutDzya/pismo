#include "utils.h"
#include <string>

namespace pismo
{

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
