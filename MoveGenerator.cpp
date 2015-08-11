#include "MoveGenerator.h"
#include "PositionState.h"
#include "PossibleMoves.h"
#include "MemPool.h"

#include <assert.h>

namespace pismo
{

MoveGenerator* MoveGenerator::_instance = 0;

MoveGenerator* MoveGenerator::instance()
{
	if (!_instance) {
		_instance = new MoveGenerator();
	}

	return _instance;
}

void MoveGenerator::destroy()
{
	delete _instance;
	_instance = 0;
}

MoveGenerator::MoveGenerator() :
	_possible_moves(new PossibleMoves())
{

}

void MoveGenerator::generate_white_moves(const PositionState& pos, moves_array& generated_moves)
{
	assert(pos.white_to_play());
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.get_board()[from / 8][from % 8]) {
			case PAWN_WHITE:
				generate_pawn_moves((Square) from, WHITE, pos, generated_moves);
				break;
			case KNIGHT_WHITE:
				generate_knight_moves((Square) from, pos, generated_moves);
				break;
			case BISHOP_WHITE:
				generate_diag_a1h8_moves((Square) from, pos, generated_moves);
				generate_diag_a8h1_moves((Square) from, pos, generated_moves);
				break;
			case ROOK_WHITE:
				generate_rank_moves((Square) from, pos, generated_moves);
				generate_file_moves((Square) from, pos, generated_moves);
				break;
			case QUEEN_WHITE:
				generate_diag_a1h8_moves((Square) from, pos, generated_moves);
				generate_diag_a8h1_moves((Square) from, pos, generated_moves);
				generate_rank_moves((Square) from, pos, generated_moves);
				generate_file_moves((Square) from, pos, generated_moves);
				break;
			case KING_WHITE:
				generate_king_moves((Square) from, pos, generated_moves);
			default:
				break;
		}
	}
}

void MoveGenerator::generate_black_moves(const PositionState& pos, moves_array& generated_moves)
{
	assert(!pos.white_to_play());
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.get_board()[from / 8][from % 8]) {
			case PAWN_BLACK:
				generate_pawn_moves((Square) from, BLACK, pos, generated_moves);
				break;
			case KNIGHT_BLACK:
				generate_knight_moves((Square) from, pos, generated_moves);
				break;
			case BISHOP_BLACK:
				generate_diag_a1h8_moves((Square) from, pos, generated_moves);
				generate_diag_a8h1_moves((Square) from, pos, generated_moves);
				break;
			case ROOK_BLACK:
				generate_rank_moves((Square) from, pos, generated_moves);
				generate_file_moves((Square) from, pos, generated_moves);
				break;
			case QUEEN_BLACK:
				generate_diag_a1h8_moves((Square) from, pos, generated_moves);
				generate_diag_a8h1_moves((Square) from, pos, generated_moves);
				generate_rank_moves((Square) from, pos, generated_moves);
				generate_file_moves((Square) from, pos, generated_moves);
				break;
			case KING_BLACK:
				generate_king_moves((Square) from, pos, generated_moves);
			default:
				break;
		}
	}
}

// Generates all legal pawn moves for color clr from square from
// and adds these moves to generated_moves data member
void MoveGenerator::generate_pawn_moves(Square from, Color clr, const PositionState& pos, moves_array& generated_moves)
{
	if (clr == WHITE) {
		const std::vector<Square>& white_pawn_moves =  _possible_moves->possible_white_pawn_moves(from);
		if(from >= A7 && from <= H7) {
			for (std::size_t move_count = 0; move_count < white_pawn_moves.size(); ++move_count) {
				move_info current_move = {from, white_pawn_moves[move_count], KNIGHT_WHITE};
				if (pos.move_is_legal(current_move)) {
					generated_moves.push_back(current_move);
					current_move.promoted = BISHOP_WHITE;
					generated_moves.push_back(current_move);
					current_move.promoted = ROOK_WHITE;
					generated_moves.push_back(current_move);
					current_move.promoted = QUEEN_WHITE;
					generated_moves.push_back(current_move);
				}
		       }
		}
		else {
			for (std::size_t move_count = 0; move_count < white_pawn_moves.size(); ++move_count) {
				move_info current_move = {from, white_pawn_moves[move_count], ETY_SQUARE};
				if (pos.move_is_legal(current_move)) {
					generated_moves.push_back(current_move);
				}
			}
		}
	}
	else {
		const std::vector<Square>& black_pawn_moves = _possible_moves->possible_black_pawn_moves(from);
		if(from >= A2 && from <= H2) {
			for (std::size_t move_count = 0; move_count < black_pawn_moves.size(); ++move_count) {
				move_info current_move = {from, black_pawn_moves[move_count], KNIGHT_BLACK};
				if (pos.move_is_legal(current_move)) {
					generated_moves.push_back(current_move);
					current_move.promoted = BISHOP_BLACK;
					generated_moves.push_back(current_move);
					current_move.promoted = ROOK_BLACK;
					generated_moves.push_back(current_move);
					current_move.promoted = QUEEN_BLACK;
					generated_moves.push_back(current_move);
				}
		       }
		}
		else {
			for (std::size_t move_count = 0; move_count < black_pawn_moves.size(); ++move_count) {
				move_info current_move = {from, black_pawn_moves[move_count], ETY_SQUARE};
				if (pos.move_is_legal(current_move)) {
					generated_moves.push_back(current_move);
				}
			}
		}
	}
}

// Generates all legal knight moves from square from
// and adds these moves to generated_moves data member
void MoveGenerator::generate_knight_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& knight_moves = _possible_moves->possible_knight_moves(from);
	for (std::size_t move_count  = 0; move_count < knight_moves.size(); ++move_count) {
		move_info current_move = {from, knight_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
	}
}

// Generates all legal king moves from square from
// and adds these moves to generated_moves data member
void MoveGenerator::generate_king_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& king_moves = _possible_moves->possible_king_moves(from);
	for (std::size_t move_count  = 0; move_count < king_moves.size(); ++move_count) {
		move_info current_move = {from, king_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
	}
}

// Generates all legal rank moves from square from
// and adds these moves to generated_moves data member
// This function should be used for rook and queen
void MoveGenerator::generate_rank_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& left_rank_moves = _possible_moves->possible_left_rank_moves(from);
	for(std::size_t move_count = 0; move_count < left_rank_moves.size(); ++move_count) {
		move_info current_move = {from, left_rank_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& right_rank_moves = _possible_moves->possible_right_rank_moves(from);
	for(std::size_t move_count = 0; move_count < right_rank_moves.size(); ++move_count) {
		move_info current_move = {from, right_rank_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal file moves from square from
// and adds these moves to generated_moves data member
// This function should be used for rook and queen
void MoveGenerator::generate_file_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& up_file_moves = _possible_moves->possible_up_file_moves(from);
	for(std::size_t move_count = 0; move_count < up_file_moves.size(); ++move_count) {
		move_info current_move = {from, up_file_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& down_file_moves = _possible_moves->possible_down_file_moves(from);
	for(std::size_t move_count = 0; move_count < down_file_moves.size(); ++move_count) {
		move_info current_move = {from, down_file_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal A1H8 diagonal moves from square from
// and adds these moves to generated_moves data member
// This function should be used for bishop and queen
void MoveGenerator::generate_diag_a1h8_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& up_diag_a1h8_moves = _possible_moves->possible_up_diag_a1h8_moves(from);
	for(std::size_t move_count = 0; move_count < up_diag_a1h8_moves.size(); ++move_count) {
		move_info current_move = {from, up_diag_a1h8_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& down_diag_a1h8_moves = _possible_moves->possible_down_diag_a1h8_moves(from);
	for(std::size_t move_count = 0; move_count < down_diag_a1h8_moves.size(); ++move_count) {
		move_info current_move = {from, down_diag_a1h8_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

// Generates all legal A8H1 diagonal moves from square from
// and adds these moves to generated_moves data member
// This function should be used for bishop and queen
void MoveGenerator::generate_diag_a8h1_moves(Square from, const PositionState& pos, moves_array& generated_moves)
{
	const std::vector<Square>& up_diag_a8h1_moves = _possible_moves->possible_up_diag_a8h1_moves(from);
	for(std::size_t move_count = 0; move_count < up_diag_a8h1_moves.size(); ++move_count) {
		move_info current_move = {from, up_diag_a8h1_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
	const std::vector<Square>& down_diag_a8h1_moves = _possible_moves->possible_down_diag_a8h1_moves(from);
	for(std::size_t move_count = 0; move_count < down_diag_a8h1_moves.size(); ++move_count) {
		move_info current_move = {from, down_diag_a8h1_moves[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			generated_moves.push_back(current_move);
		}
		if (pos.get_board()[current_move.to / 8][current_move.to % 8] != ETY_SQUARE) {
			break;
		}
	}
}

MoveGenerator::~MoveGenerator()
{
	delete _possible_moves;
}

}
