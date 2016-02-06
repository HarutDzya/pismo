#ifndef _SCORE_H_
#define _SCORE_H_

namespace pismo
{

//http://www.talkchess.com/forum/viewtopic.php?t=42054

struct Score
{
  int16_t _mgScore;
  int16_t _egScore;

  Score() :
      _mgScore(0),
      _egScore(0)
  {
  }

  Score(int16_t mg, int16_t eg) :
      _mgScore(mg),
      _egScore(eg)
  {
  }

  void setScore(int16_t mg, int16_t eg)
  {
    _mgScore = mg;
    _egScore = eg;
  }

  const Score operator / (int16_t d)
  {
		return Score(_mgScore / d, _egScore / d);
  }

	const Score& operator = (const Score& s)
	{
		_mgScore = s._mgScore;
		_egScore = s._egScore;
		return s;
	}
};

inline void addScore(Score& s, int16_t mg, int16_t eg)
{
  s._mgScore += mg;
  s._egScore += eg;
}

inline void subScore(Score& s, int16_t mg, int16_t eg)
{
  s._mgScore -= mg;
  s._egScore -= eg;
}

inline void operator += (Score& s, int16_t d)
{
  s._mgScore = s._mgScore + d;
  s._egScore = s._egScore + d;
}

inline void operator += (Score& s1, Score& s2)
{
  s1._mgScore = s1._mgScore + s2._mgScore;
  s1._egScore = s1._egScore + s2._mgScore;
}

inline void operator -= (Score& s, int16_t d)
{
  s._mgScore = s._mgScore - d;
  s._egScore = s._egScore - d;
}

inline void operator -= (Score& s1, Score& s2)
{
  s1._mgScore = s1._mgScore - s2._mgScore;
  s1._egScore = s1._egScore - s2._mgScore;
}

inline void operator *= (Score& s, int16_t d)
{
  s._mgScore = s._mgScore * d;
  s._egScore = s._egScore * d;
}

inline void operator /= (Score& s, int16_t d)
{
  s._mgScore = s._mgScore / d;
  s._egScore = s._egScore / d;
}

inline Score operator + (Score s1, Score& s2)
{
  return Score(s1._mgScore + s2._mgScore, s1._egScore + s2._egScore);
}

inline Score operator + (int16_t d, Score& s)
{
  return Score(s._mgScore + d, s._egScore + d);
}

inline Score operator + (Score& s, int16_t d)
{
  return Score(s._mgScore + d, s._egScore + d);
}

inline Score operator - (Score s1, Score& s2)
{
  return Score(s1._mgScore - s2._mgScore, s1._egScore - s2._egScore);
}

inline Score operator - (int16_t d, Score& s)
{
  return Score(s._mgScore - d, s._egScore - d);
}

inline Score operator - (Score& s, int16_t d)
{
  return Score(s._mgScore - d, s._egScore - d);
}

inline Score operator * (Score& s, int16_t d)
{
  return Score(d * s._mgScore, d * s._egScore);
}

inline Score operator * (int16_t d, Score& s)
{
  return Score(d * s._mgScore, d * s._egScore);
}
	
}

#endif // _SCORE_H_
