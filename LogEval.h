#include <iostream>

#define incr(a, b, c) log::incr_##c(a, b)
#define decr(a, b, c) log::decr_##c(a, b)
#define printEvalLog log::print
#define resetEvalLog log::reset

namespace log
{

struct EvalLog
{
  int16_t _pst;
  int16_t _material;
  int16_t _whiteMobility;
  int16_t _blackMobility;

} eLog;


void incr_pst(int16_t& a, int16_t b)
{
  a += b;
  eLog._pst += b;
}

void incr_material(int16_t& a, int16_t b)
{
  a += b;
  eLog._material += b;
}

void incr_whiteMobility(int16_t& a, int16_t b)
{
  a += b;
  eLog._whiteMobility += b;
}

void incr_blackMobility(int16_t& a, int16_t b)
{
  a += b;
  eLog._whiteMobility += b;
}

void decr_whiteMobility(int16_t& a, int16_t b)
{
  a -= b;
  eLog._blackMobility += b;
}

void decr_blackMobility(int16_t& a, int16_t b)
{
  a -= b;
  eLog._blackMobility += b;
}

void print()
{
  std::cout << "\n\nPST            : " << eLog._pst << std::endl;
  std::cout << "Material       : " << eLog._material << std::endl;
  std::cout << "White Mobility : " << eLog._whiteMobility << std::endl;
  std::cout << "Black Mobility : " << eLog._blackMobility << std::endl;

  int16_t total = eLog._pst + eLog._material + eLog._whiteMobility - eLog._blackMobility;
  std::cout << "\n\tTotal : " << total << "\n\n" << std::endl;
}

void reset()
{
  eLog._pst = 0;
  eLog._material = 0;
  eLog._whiteMobility = 0;
  eLog._blackMobility = 0;
}


}
