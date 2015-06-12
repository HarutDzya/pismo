#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class PositionState;
class PossibleMoves;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	const std::vector<move_info>& generate_white_moves(const PositionState& pos);
	const std::vector<move_info>& generate_black_moves(const PositionState& pos);

private:
	MoveGenerator();
	~MoveGenerator();
	static MoveGenerator* _instance;

	void generate_pawn_moves(Square from, Color clr, const PositionState& pos);
	void generate_knight_moves(Square from, const PositionState& pos);
	void generate_king_moves(Square from, const PositionState& pos);
	void generate_rank_moves(Square from, const PositionState& pos);
	void generate_file_moves(Square from, const PositionState& pos);
	void generate_diag_a1h8_moves(Square from, const PositionState& pos);
	void generate_diag_a8h1_moves(Square from, const PositionState& pos);

	const PossibleMoves* _possible_moves;
	std::vector<move_info> _generated_moves;
};

}

#endif
