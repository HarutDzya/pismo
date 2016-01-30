#include <iostream>
#include "Score.h"

#define incr(a, b, c) log::incr_##c(a, b)
#define decr(a, b, c) log::decr_##c(a, b)

#define incrScore(a, b, c, d) log::incrScore_##d(a, b, c)
#define decrScore(a, b, c, d) log::decrScore_##d(a, b, c)

#define printEvalLog log::print
#define resetEvalLog log::reset

namespace log
{
using namespace pismo;

struct EvalLog
{
  int16_t _material;
  Score _pst;
  Score _whiteMobility;
  Score _blackMobility;

} eLog;


void incr_material(int16_t& a, int16_t b)
{
  a += b;
  eLog._material += b;
}


void incrScore_pst(Score& s, int16_t b, int16_t c)
{
  s._mgScore += b;
  s._egScore += c;
  eLog._pst._mgScore += b;
  eLog._pst._egScore += c;
}

void incrScore_whiteMobility(Score& s, int16_t b, int16_t c)
{
  s._mgScore += b;
  s._egScore += c;
  eLog._whiteMobility._mgScore += b;
  eLog._whiteMobility._egScore += c;
}

void incrScore_blackMobility(Score& s, int16_t b, int16_t c)
{
  s._mgScore += b;
  s._egScore += c;
  eLog._whiteMobility._mgScore += b;
  eLog._whiteMobility._egScore += c;
}

void decrScore_whiteMobility(Score& s, int16_t b, int16_t c)
{
  s._mgScore -= b;
  s._egScore -= c;
  eLog._blackMobility._mgScore += b;
  eLog._blackMobility._egScore += c;
}

void decrScore_blackMobility(Score& s, int16_t b, int16_t c)
{
  s._mgScore -= b;
  s._egScore -= c;
  eLog._blackMobility._mgScore += b;
  eLog._blackMobility._egScore += c;
}

void print(uint8_t phase)
{
  std::cout << "\n\nMaterial       : " << eLog._material << std::endl;
  std::cout << "PST            : " << eLog._pst._mgScore << "  " << eLog._pst._egScore<< std::endl;
  std::cout << "White Mobility : " << eLog._whiteMobility._mgScore << "  " <<  eLog._whiteMobility._egScore << std::endl;
  std::cout << "Black Mobility : " << eLog._blackMobility._mgScore << "  " << eLog._blackMobility._egScore << std::endl;

  Score s = (eLog._whiteMobility - eLog._blackMobility);
  s += eLog._pst ;
  int16_t value = ( s._mgScore * phase  + s._egScore * (128 - (int)phase) ) / 128 + eLog._material;

  std::cout << "\n\tTotal : " << value << "\n\n" << std::endl;
}

void reset()
{
  eLog._material = 0;
  eLog._pst.setScore(0, 0);
  eLog._whiteMobility.setScore(0, 0);
  eLog._blackMobility.setScore(0, 0);
}


}
