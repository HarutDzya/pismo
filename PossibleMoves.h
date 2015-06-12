#ifndef _POSSIBLE_MOVES_H_
#define _POSSIBLE_MOVES_H_

#include "utils.h"
#include <vector>

namespace pismo
{

class PossibleMoves
{
public:
	PossibleMoves();

	const std::vector<Square>& possible_white_pawn_moves(Square from) const;
	const std::vector<Square>& possible_black_pawn_moves(Square from) const;
	const std::vector<Square>& possible_knight_moves(Square from) const;
	const std::vector<Square>& possible_king_moves(Square from) const;
	const std::vector<Square>& possible_left_rank_moves(Square from) const;
	const std::vector<Square>& possible_right_rank_moves(Square from) const;
	const std::vector<Square>& possible_up_file_moves(Square from) const;
	const std::vector<Square>& possible_down_file_moves(Square from) const;
	const std::vector<Square>& possible_up_diag_a1h8_moves(Square from) const;
	const std::vector<Square>& possible_down_diag_a1h8_moves(Square from) const;
	const std::vector<Square>& possible_up_diag_a8h1_moves(Square from) const;
	const std::vector<Square>& possible_down_diag_a8h1_moves(Square from) const;


private:
	void init_white_pawn_moves();
	void init_black_pawn_moves();
	void init_knight_moves();
	void init_king_moves();
	void init_left_rank_moves();
	void init_right_rank_moves();
	void init_up_file_moves();
	void init_down_file_moves();
	void init_up_diag_a1h8_moves();
	void init_down_diag_a1h8_moves();
	void init_up_diag_a8h1_moves();
	void init_down_diag_a8h1_moves();

private:
	std::vector<std::vector<Square> > _white_pawn_moves_list;
	std::vector<std::vector<Square> > _black_pawn_moves_list;
	std::vector<std::vector<Square> > _knight_moves_list;
	std::vector<std::vector<Square> > _king_moves_list;
	std::vector<std::vector<Square> > _left_rank_moves_list;
	std::vector<std::vector<Square> > _right_rank_moves_list;
	std::vector<std::vector<Square> > _up_file_moves_list;
	std::vector<std::vector<Square> > _down_file_moves_list;
	std::vector<std::vector<Square> > _up_diag_a1h8_moves_list;
	std::vector<std::vector<Square> > _down_diag_a1h8_moves_list;
	std::vector<std::vector<Square> > _up_diag_a8h1_moves_list;
	std::vector<std::vector<Square> > _down_diag_a8h1_moves_list;
};

}

#endif
