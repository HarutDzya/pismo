#include "Core.h"
#include "MoveGenerator.h"
#include "PositionEvaluation.h"
#include "utils.h"
#include <vector>

namespace pismo
{

move_info Core::think(PositionState& pos, uint16_t depth, bool white_to_play)
{
  std::vector<move_info>& possibleMoves = white_to_play ?
          MoveGenerator::instance().generate_white_moves(pos) : MoveGenerator::instance().generate_black_moves(pos); 

  if (possibleMoves.empty())
  {
    return move_info(); //mate
  }

  move_info move = possibleMoves[0];
  if (possibleMoves.size() == 1) return move;

  if (white_to_play)
  {
    float score = -MAX_SCORE;
    for (uint16_t i = 0; i < possibleMoves.size(); ++i)
    {
      pos.make_move(possibleMoves[i]);
      float s = minimax(pos, depth - 1, !white_to_play);
      pos.undo_move();
      if (s > score)
      {
        s = score;
        move = possibleMoves[i];
      }
    }
    return score;
  }
  else
  {
    float score = MAX_SCORE;
    for (uint16_t i = o; i < possibleMoves.size(); ++i)
    {
      pos.make_move(possibleMoves[i]);
      float s = minimax(pos, depth - 1, !white_to_play);
      pos.undo_move();
      if (s < score)
      {
        s = score;
        move = possibleMoves[i];
      }
    }
    return score;
  }

  return move;
}

float Core::minimax(PositionState& pos, uint16_t depth, bool white_to_play)
{
  if (depth = 0)
  {
    return PositionEvaluation::instance().evaluate(pos);
  }

  std::vector<move_info>& possibleMoves = white_to_play ?
          MoveGenerator::instance().generate_white_moves(pos) : MoveGenerator::instance().generate_black_moves(pos); 
  if (possibleMoves.empty())
  {
    return white_to_play ? -MAX_SCORE : MAX_SCORE;
  }

  if (white_to_play)
  {
    float score = -MAX_SCORE;
    for (uint16_t i = 0; i < possibleMoves.size(); ++i)
    {
      pos.make_move(possibleMoves[i]);
      float s = minimax(pos, depth - 1, !white_to_play);
      pos.undo_move();
      if (s > score) score = s;
    }
    return score;
  }
  else
  {
    float score = MAX_SCORE;
    for (uint16_t i = o; i < possibleMoves.size(); ++i)
    {
      pos.make_move(possibleMoves[i]);
      float s = minimax(pos, depth - 1, !white_to_play);
      pos.undo_move();
      if (s < score) score = s;
    }
    return score;
  }
}

Core::Core()
{
}

Core::~Core()
{
}

}
