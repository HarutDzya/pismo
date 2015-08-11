#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"

namespace pismo
{

class PositionState;
class PossibleMoves;
struct moves_array;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	void generate_white_moves(const PositionState& pos, moves_array& generated_moves);
	void generate_black_moves(const PositionState& pos, moves_array& generated_moves);

private:
	MoveGenerator();
	~MoveGenerator();
	static MoveGenerator* _instance;

	void generate_pawn_moves(Square from, Color clr, const PositionState& pos, moves_array& generated_moves);
	void generate_knight_moves(Square from, const PositionState& pos, moves_array& generated_moves);
	void generate_king_moves(Square from, const PositionState& pos, moves_array& generated_moves);
	void generate_rank_moves(Square from, const PositionState& pos, moves_array& generated_moves);
	void generate_file_moves(Square from, const PositionState& pos, moves_array& generated_moves);
	void generate_diag_a1h8_moves(Square from, const PositionState& pos, moves_array& generated_moves);
	void generate_diag_a8h1_moves(Square from, const PositionState& pos, moves_array& generated_moves);

	const PossibleMoves* _possible_moves;
};

}

#endif
