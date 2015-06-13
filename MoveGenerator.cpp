#include "MoveGenerator.h"
#include "PositionState.h"
#include "PossibleMoves.h"

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

const std::vector<move_info>& MoveGenerator::generate_white_moves(const PositionState& pos)
{
	assert(pos.white_to_play());
	_generated_moves.clear();
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.get_board()[from / 8][from % 8]) {
			case PAWN_WHITE:
				generate_pawn_moves((Square) from, WHITE, pos);
				break;
			case KNIGHT_WHITE:
				generate_knight_moves((Square) from, pos);
				break;
			case BISHOP_WHITE:
				generate_diag_a1h8_moves((Square) from, pos);
				generate_diag_a8h1_moves((Square) from, pos);
				break;
			case ROOK_WHITE:
				generate_rank_moves((Square) from, pos);
				generate_file_moves((Square) from, pos);
				break;
			case QUEEN_WHITE:
				generate_diag_a1h8_moves((Square) from, pos);
				generate_diag_a8h1_moves((Square) from, pos);
				generate_rank_moves((Square) from, pos);
				generate_file_moves((Square) from, pos);
				break;
			case KING_WHITE:
				generate_king_moves((Square) from, pos);
		}
	}
	return _generated_moves;
}

const std::vector<move_info>& MoveGenerator::generate_black_moves(const PositionState& pos)
{
	assert(!pos.white_to_play());
	_generated_moves.clear();
	for (unsigned int from = A1; from <= H8; ++from) {
		switch(pos.get_board()[from / 8][from % 8]) {
			case PAWN_BLACK:
				generate_pawn_moves((Square) from, BLACK, pos);
				break;
			case KNIGHT_BLACK:
				generate_knight_moves((Square) from, pos);
				break;
			case BISHOP_BLACK:
				generate_diag_a1h8_moves((Square) from, pos);
				generate_diag_a8h1_moves((Square) from, pos);
				break;
			case ROOK_BLACK:
				generate_rank_moves((Square) from, pos);
				generate_file_moves((Square) from, pos);
				break;
			case QUEEN_BLACK:
				generate_diag_a1h8_moves((Square) from, pos);
				generate_diag_a8h1_moves((Square) from, pos);
				generate_rank_moves((Square) from, pos);
				generate_file_moves((Square) from, pos);
				break;
			case KING_BLACK:
				generate_king_moves((Square) from, pos);
		}
	}
	return _generated_moves;
}

// Generates all legal pawn moves for color clr from square from
// and adds these moves to _generated_moves data member
void MoveGenerator::generate_pawn_moves(Square from, Color clr, const PositionState& pos)
{
	if (clr == WHITE) {
		if(from >= A7 && from <= H7) {
			for (std::size_t move_count = 0; move_count < _possible_moves->possible_white_pawn_moves(from).size(); ++move_count) {
				move_info current_move = {from, (_possible_moves->possible_white_pawn_moves(from))[move_count], KNIGHT_WHITE};
				if (pos.move_is_legal(current_move)) {
					_generated_moves.push_back(current_move);
					current_move.promoted = BISHOP_WHITE;
					_generated_moves.push_back(current_move);
					current_move.promoted = ROOK_WHITE;
					_generated_moves.push_back(current_move);
					current_move.promoted = QUEEN_WHITE;
					_generated_moves.push_back(current_move);
				}
		       }
		}
		else {
			for (std::size_t move_count = 0; move_count < _possible_moves->possible_white_pawn_moves(from).size(); ++move_count) {
				move_info current_move = {from, (_possible_moves->possible_white_pawn_moves(from))[move_count], ETY_SQUARE};
				if (pos.move_is_legal(current_move)) {
					_generated_moves.push_back(current_move);
				}
			}
		}
	}
	else {
		if(from >= A2 && from <= H2) {
			for (std::size_t move_count = 0; move_count < _possible_moves->possible_black_pawn_moves(from).size(); ++move_count) {
				move_info current_move = {from, (_possible_moves->possible_black_pawn_moves(from))[move_count], KNIGHT_BLACK};
				if (pos.move_is_legal(current_move)) {
					_generated_moves.push_back(current_move);
					current_move.promoted = BISHOP_BLACK;
					_generated_moves.push_back(current_move);
					current_move.promoted = ROOK_BLACK;
					_generated_moves.push_back(current_move);
					current_move.promoted = QUEEN_BLACK;
					_generated_moves.push_back(current_move);
				}
		       }
		}
		else {
			for (std::size_t move_count = 0; move_count < _possible_moves->possible_black_pawn_moves(from).size(); ++move_count) {
				move_info current_move = {from, (_possible_moves->possible_black_pawn_moves(from))[move_count], ETY_SQUARE};
				if (pos.move_is_legal(current_move)) {
					_generated_moves.push_back(current_move);
				}
			}
		}
	}
}

// Generates all legal knight moves from square from
// and adds these moves to _generated_moves data member
void MoveGenerator::generate_knight_moves(Square from, const PositionState& pos)
{
	for (std::size_t move_count  = 0; move_count < _possible_moves->possible_knight_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_knight_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
	}
}

// Generates all legal king moves from square from
// and adds these moves to _generated_moves data member
void MoveGenerator::generate_king_moves(Square from, const PositionState& pos)
{
	for (std::size_t move_count  = 0; move_count < _possible_moves->possible_king_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_king_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
	}
}

// Generates all legal rank moves from square from
// and adds these moves to _generated_moves data member
// This function should be used for rook and queen
void MoveGenerator::generate_rank_moves(Square from, const PositionState& pos)
{
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_left_rank_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_left_rank_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_right_rank_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_right_rank_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
}

// Generates all legal file moves from square from
// and adds these moves to _generated_moves data member
// This function should be used for rook and queen
void MoveGenerator::generate_file_moves(Square from, const PositionState& pos)
{
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_up_file_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_up_file_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_down_file_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_down_file_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
}

// Generates all legal A1H8 diagonal moves from square from
// and adds these moves to _generated_moves data member
// This function should be used for bishop and queen
void MoveGenerator::generate_diag_a1h8_moves(Square from, const PositionState& pos)
{
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_up_diag_a1h8_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_up_diag_a1h8_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_down_diag_a1h8_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_down_diag_a1h8_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
}

// Generates all legal A8H1 diagonal moves from square from
// and adds these moves to _generated_moves data member
// This function should be used for bishop and queen
void MoveGenerator::generate_diag_a8h1_moves(Square from, const PositionState& pos)
{
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_up_diag_a8h1_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_up_diag_a8h1_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
	for(std::size_t move_count = 0; move_count < _possible_moves->possible_down_diag_a8h1_moves(from).size(); ++move_count) {
		move_info current_move = {from, (_possible_moves->possible_down_diag_a8h1_moves(from))[move_count], ETY_SQUARE};
		if (pos.move_is_legal(current_move)) {
			_generated_moves.push_back(current_move);
		}
		else {
			break;
		}
	}
}

MoveGenerator::~MoveGenerator()
{
	delete _possible_moves;
}

}
