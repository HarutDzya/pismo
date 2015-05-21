#include "PositionState.h"
#include <assert.h>
#include <cstdlib>
#include <iostream>

namespace pismo
{

PositionState::PositionState(): 
_white_pieces(0), 
_white_pieces_transpose(0),
_white_pieces_diag_a1h8(0),
_white_pieces_diag_a8h1(0),
_black_pieces(0),
_black_pieces_transpose(0),
_black_pieces_diag_a1h8(0),
_black_pieces_diag_a8h1(0),
_bitboard_impl(new BitboardImpl()),
_white_to_play(true),
_king_under_attack(false),
_en_passant_file(-1),
_white_left_castling(true),
_white_right_castling(true),
_black_left_castling(true),
_black_right_castling(true)
{
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			_board[i][j] = ETY_SQUARE;
		}
	}
	for (int i = 0; i < PIECE_NB / 2; ++i) {
		_white_pieces_count[i] = 0;
		_black_pieces_count[i] = 0;
	} 
}

PositionState::~PositionState()
{
	delete _bitboard_impl;
}

void PositionState::set_piece(Square s, Piece p)
{
	_board[s / 8][s % 8] = p;
	Bitboard tmp = 1;
	if (p <= KING_WHITE) {
		add_piece_to_bitboards(s, WHITE);
	}
	else {
		add_piece_to_bitboards(s, BLACK);
	}
}

void PositionState::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	if (init_position_is_valid(pieces)) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			set_piece(pieces[i].first, pieces[i].second);
			if (pieces[i].second <= KING_WHITE) {
				++(_white_pieces_count[pieces[i].second]);
			}
			else {
				++(_black_pieces_count[pieces[i].second % (PIECE_NB / 2)]);
			}
		}
	}
}
/* Checks for the following conditions for initial position validty:
	1) That there are no pawns on the first and last ranks
	2) For each square there is at most one entry
	3) Number of pieces for each color is not more than 16	
	4) Number of pawns for each color is not more than 8
	5) Each color has only one king
	6) Number of promoted pieces for each color is not more than the missing pawns 
*/
bool PositionState::init_position_is_valid(const std::vector<std::pair<Square, Piece> >& pieces) const
{
	Piece board[NUMBER_OF_SQUARES];
	for (unsigned int sq = A1; sq < NUMBER_OF_SQUARES; ++sq) {
		board[sq] = ETY_SQUARE;
	} 
	Count pieces_count[PIECE_NB] = {};
	Count white_pieces_sum = 0;
	Count black_pieces_sum = 0;
	for (std::size_t i = 0; i < pieces.size(); ++i) {
		assert(pieces[i].first >= A1 && pieces[i].first <= H8);
		assert(pieces[i].second >= PAWN_WHITE && pieces[i].second <= KING_BLACK);
		if ((pieces[i].second == PAWN_WHITE || pieces[i].second == PAWN_BLACK) &&
		(pieces[i].first <= H1 || pieces[i].first >= A8)) {
			return false;
		}
		else {
			if (pieces[i].second <= KING_WHITE) {
				++pieces_count[pieces[i].second];
				++white_pieces_sum;
			}
			else {
				++pieces_count[pieces[i].second];
				++black_pieces_sum;
			}
		}
		
		if (board[pieces[i].first] != ETY_SQUARE) {
			return false;
		}
		else {
			board[pieces[i].first] = pieces[i].second;
		}
	}
	
	if (white_pieces_sum > 16 || black_pieces_sum > 16) {
		return false;
	}

	Count white_promoted_pieces_sum = 0;
	Count black_promoted_pieces_sum = 0;

	for (unsigned int piece = PAWN_WHITE; piece <= KING_BLACK; ++piece) {
		switch(piece) {
			case PAWN_WHITE: 
				if (pieces_count[piece] > 8) {
					return false;
				}
				break;
			case PAWN_BLACK:
				if (pieces_count[piece] > 8) {
					return false;
				}
				break;
			case KING_WHITE:
				if (pieces_count[piece] != 1) {
					return false;
				}
				break;
			case KING_BLACK:
				if (pieces_count[piece] != 1) {
					return false;
				}
				break;
			case QUEEN_WHITE:
				white_promoted_pieces_sum += (pieces_count[piece] - 1) > 0 ? pieces_count[piece] - 1 : 0;
				break;
			case QUEEN_BLACK:
				black_promoted_pieces_sum += (pieces_count[piece] - 1) > 0 ? pieces_count[piece] - 1 : 0;
				break;
			default:
				if (piece < KING_WHITE) {
					white_promoted_pieces_sum += (pieces_count[piece] - 2) > 0 ? pieces_count[piece] - 2 : 0;
				}
				else {
				 	black_promoted_pieces_sum += (pieces_count[piece] - 2) > 0 ? pieces_count[piece] - 2 : 0;
				}
		}
	}
	
	if ((white_promoted_pieces_sum > 8 - pieces_count[PAWN_WHITE]) || (black_promoted_pieces_sum > 8 - pieces_count[PAWN_BLACK])) {
		return false;
	}
	
	return true; 
}

bool PositionState::move_is_legal(const move_info& move) const
{
	Piece pfrom = _board[move.from  / 8][move.from % 8];
	Piece pto = _board[move.to  / 8][move.to % 8];
	if(pfrom == ETY_SQUARE) {
		return false;
	}

	if(_white_to_play) {
		if((_bitboard_impl->square_to_bitboard(move.from) & _black_pieces) || (_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_WHITE: 
					return pawn_move_is_legal(move);
				case KNIGHT_WHITE:
					return knight_move_is_legal(move);
				case BISHOP_WHITE:
					return bishop_move_is_legal(move);
				case ROOK_WHITE:
					return rook_move_is_legal(move);
				case QUEEN_WHITE:
					return queen_move_is_legal(move);
				case KING_WHITE:
					if (std::abs(move.from % 8 - move.to % 8) > 1) { 
						return castling_is_legal(move);
					}
					else { 
						return king_move_is_legal(move);
					}
				default: 
					return false;
			}
		}
	}
	else {
		if((_bitboard_impl->square_to_bitboard(move.from) & _white_pieces) || (_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_BLACK: 
					return pawn_move_is_legal(move);
				case KNIGHT_BLACK: 
					return knight_move_is_legal(move);
				case BISHOP_BLACK:
					return bishop_move_is_legal(move);
				case ROOK_BLACK:
					return rook_move_is_legal(move);
				case QUEEN_BLACK:
					return queen_move_is_legal(move);
				case KING_BLACK:
					if (std::abs(move.from % 8 - move.to % 8) > 1) { 
						return castling_is_legal(move);
					}
					else { 
						return king_move_is_legal(move);
					}
				default: 
					return false;
			}
		}
	}

}

bool PositionState::pawn_move_is_legal(const move_info& move) const
{
	bool result = false; 
	if (_white_to_play) {
		assert(move.from > H1);
		// Checks for single square movement of the pawn
		if (move.to - move.from == 8 && !(_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			result = true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.to - move.from == 16 && (_bitboard_impl->square_to_bitboard(move.from) & PAWN_WHITE_INIT) &&
		!(_bitboard_impl->square_to_bitboard((Square)(move.from + 8)) & (_white_pieces | _black_pieces)) && !(_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			result = true;
		}
		// Checks for usual capture movement of the pawn, the last condition checks for edge capture
		else if ((move.to - move.from == 7 || move.to - move.from == 9) && (_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)
		&& (move.to / 8 - move.from / 8) == 1) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			result = true;
		}
		else if (en_passant_capture_is_legal(move)) {
			result = true;
		}
	}
	else {
		// Checks for single square movement of the pawn
		assert(move.from < A8);
		if (move.from - move.to == 8 && !(_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			result = true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.from - move.to == 16 && (_bitboard_impl->square_to_bitboard(move.from) & PAWN_BLACK_INIT) &&
		!(_bitboard_impl->square_to_bitboard((Square)(move.from - 8)) & (_white_pieces | _black_pieces)) && !(_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			result = true;
		}
		// Checks for usual capture movement of the pawn, the last condition checks for edge capture
		else if ((move.from - move.to == 7 || move.from - move.to == 9) && (_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)
		&& (move.from / 8 - move.to / 8) == 1) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			result = true;
		}
		else if (en_passant_capture_is_legal(move)) {
			result = true;
		}
	}
	// TODO: Check that after this move the king is not under attack
	// and update result variable appropriately

	return result;
}

bool PositionState::en_passant_capture_is_legal(const move_info& move) const
{
	if (_en_passant_file != -1) {
		if (_white_to_play) {
			assert(_board[4][_en_passant_file] == PAWN_BLACK);
			if (_en_passant_file == 0 && move.from == B5 && move.to == A6) {
				return true;
			}
			else if (_en_passant_file == 7 && move.from == G5 && move.to == H6) {
				return true;
			} 
			else if ((move.from == 4 * 8 + _en_passant_file + 1) || (move.from == 4 * 8 + _en_passant_file - 1)
			&& move.to == 5 * 8 + _en_passant_file) {
				return true;
			}
		}
		else {
			assert(_board[3][_en_passant_file] == PAWN_WHITE);
			if (_en_passant_file == 0 && move.from == B4 && move.to == A3) {
				return true;
			}
			else if (_en_passant_file == 7 && move.from == G4 && move.to == H3) {
				return true;
			}
			else if ((move.from == 3 * 8 + _en_passant_file + 1) || (move.from == 3 * 8 + _en_passant_file - 1)
			&& move.to == 2 * 8 + _en_passant_file) {
				return true;
			}
		}
 	}
	
	return false;
}

bool PositionState::knight_move_is_legal(const move_info& move) const
{
	switch (move.from % 8) {
		case 0: 
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= A3 ? (KNIGHT_MOVES_A3 >> A3 - move.from) :
			(KNIGHT_MOVES_A3 << move.from - A3))) {
				return true;
			}
			break;
		case 1:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= B3 ? (KNIGHT_MOVES_B3 >> B3 - move.from) :
			(KNIGHT_MOVES_B3 << move.from - B3))) {
				return true; 	
			}
			break;
		case 6:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= G3 ? (KNIGHT_MOVES_G3 >> G3 - move.from) :
			(KNIGHT_MOVES_G3 << move.from - G3))) {
				return true;
			}
			break;
		case 7:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= H3 ? (KNIGHT_MOVES_H3 >> H3 - move.from) :
			(KNIGHT_MOVES_H3 << move.from - H3))) {
				return true;
			}
			break;
		default:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= C3 ? (KNIGHT_MOVES_C3 >> C3 - move.from) :
			(KNIGHT_MOVES_C3 << move.from - C3))) {
				return true;
			}
	}
	return false;
}

bool PositionState::bishop_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _bitboard_impl->get_legal_diag_a1h8_moves(move.from, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8))
	|| (_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _bitboard_impl->get_legal_diag_a8h1_moves(move.from, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1))) {
		return true;
	}
	else {
		return false;
	}
}

bool PositionState::rook_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_rank_moves(move.from, _white_pieces | _black_pieces))
	|| (_bitboard_impl->square_to_bitboard_transpose(move.to) & _bitboard_impl->get_legal_file_moves(move.from, _white_pieces_transpose | _black_pieces_transpose))) {
		return true;
	}
	else {
		return false;
	}
}

bool PositionState::queen_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_rank_moves(move.from, _white_pieces | _black_pieces))
	|| (_bitboard_impl->square_to_bitboard_transpose(move.to) & _bitboard_impl->get_legal_file_moves(move.from, _white_pieces_transpose | _black_pieces_transpose)) 
	|| (_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _bitboard_impl->get_legal_diag_a1h8_moves(move.from, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8))
	|| (_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _bitboard_impl->get_legal_diag_a8h1_moves(move.from, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1))) {
		return true;
	}
	else {
		return false;
	}
}

bool PositionState::king_move_is_legal(const move_info& move) const
{
	switch (move.from % 8) {
		case 0: 
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= A2 ? (KING_MOVES_A2 >> A2 - move.from) :
			(KING_MOVES_A2 << move.from - A2))) {
				return true;
			}
			break;
		case 7:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= H2 ? (KING_MOVES_H2 >> H2 - move.from) :
			(KING_MOVES_H2 << move.from - H2))) {
				return true;
			}
			break;
		default:
			if (_bitboard_impl->square_to_bitboard(move.to) & (move.from <= B2 ? (KING_MOVES_B2 >> B2 - move.from) :
			(KING_MOVES_B2 << move.from - B2))) {
				return true;
			}
	}
	return false;
}

bool PositionState::castling_is_legal(const move_info& move) const
{
	return true;
}

void PositionState::make_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (pfrom == PAWN_WHITE || pfrom == PAWN_BLACK) {
		if(move.to >= A8 || move.to <= H1) {
			make_promotion_move(move);
		}
		else if (std::abs(move.from - move.to) == 16) {
			make_en_passant_move(move);
		}
		else if (((move.to - move.from) % 8) && pto == ETY_SQUARE) {
			make_en_passant_capture(move);
		}
		else {
			make_normal_move(move);
		}
	}
	else if ((pfrom == KING_WHITE || pfrom == KING_BLACK) && std::abs(move.from % 8 - move.to % 8) > 1) {
		make_castling_move(move);
	}		
	else {
		make_normal_move(move);
	}

	
	update_castling_rights();
	// todo : check if opponents king is under attack and update the variable accordingly
	
	_white_to_play = !_white_to_play;
}

void PositionState::make_normal_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {			
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, BLACK);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, WHITE);
			--(_white_pieces_count[_board[move.to / 8][move.to % 8]]);
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = -1;				
}

//Castling is assumed to be King's move
void PositionState::make_castling_move(const move_info& move)
{
	if (_white_to_play) {
		if (move.from < move.to) {
			_white_pieces ^= WHITE_RIGHT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8] = KING_WHITE;
			_board[move.to / 8][move.to % 8 + 1] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 - 1] = ROOK_WHITE;
		}	
		else {
			_white_pieces ^= WHITE_LEFT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8] = KING_WHITE;
			_board[move.to / 8][move.to % 8 - 2] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 + 1] = ROOK_WHITE;
		}
	}
	else {
		if (move.from < move.to) {
			_black_pieces ^= BLACK_RIGHT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8] = KING_BLACK;
			_board[move.to / 8][move.to % 8 + 1] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 - 1] = ROOK_BLACK;
		}	
		else {
			_black_pieces ^= BLACK_LEFT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8] = KING_BLACK;
			_board[move.to / 8][move.to % 8 - 2] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 + 1] = ROOK_BLACK;
		}
	}
}

void PositionState::make_en_passant_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
	}	
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = move.from % 8;					
}

void PositionState::make_en_passant_capture(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		remove_piece_from_bitboards((Square) (move.to - 8), BLACK);
		--(_black_pieces_count[PAWN_BLACK % (PIECE_NB / 2)]);
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		remove_piece_from_bitboards((Square) (move.to + 8), WHITE);
		--(_white_pieces_count[PAWN_WHITE]);
		_board[move.to / 8 + 1][move.to % 8] = ETY_SQUARE;
		
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = -1;
}

void PositionState::make_promotion_move(const move_info& move)
{
	Bitboard tmp = 1;
	if (_white_to_play) {
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, BLACK);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
		--(_white_pieces_count[PAWN_WHITE]);
		++(_white_pieces_count[move.promoted]);
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, WHITE);	
			--(_white_pieces_count[_board[move.to / 8][move.to % 8]]);
		}
		--(_black_pieces_count[PAWN_BLACK % (PIECE_NB / 2)]);
		++(_black_pieces_count[move.promoted % (PIECE_NB / 2)]);
		
		}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = move.promoted;
	_en_passant_file = -1;
}

void PositionState::update_castling_rights()
{
	if (_white_to_play) {
		if (_white_left_castling || _white_right_castling) {
			if (_board[E1 / 8][E1 % 8] != KING_WHITE){
				_white_left_castling = false;
				_white_right_castling = false;
			}
			else {
				if (_board[A1 / 8][A1 % 8] != ROOK_WHITE) {
					_white_left_castling = false;
				}
				if (_board[H1 / 8][H1 % 8] != ROOK_WHITE) {
					_white_right_castling = false;
				}
			}
		}
	}
	else {
		if (_black_left_castling || _black_right_castling) {
			if (_board[E8 / 8][E8 % 8] != KING_BLACK){
				_black_left_castling = false;
				_black_right_castling = false;
			}
			else {
				if (_board[A8 / 8][A8 % 8] != ROOK_BLACK) {
					_black_left_castling = false;
				}
				if (_board[H8 / 8][H8 % 8] != ROOK_BLACK) {
					_black_right_castling = false;
				}
			}
		}
	}
}

void PositionState::add_piece_to_bitboards(Square sq, Color clr)
{
	assert(clr == WHITE || clr == BLACK);
	if (clr == WHITE) {
		_white_pieces |= _bitboard_impl->square_to_bitboard(sq);
		_white_pieces_transpose |= _bitboard_impl->square_to_bitboard_transpose(sq);
		_white_pieces_diag_a1h8 |= _bitboard_impl->square_to_bitboard_diag_a1h8(sq);
		_white_pieces_diag_a8h1 |= _bitboard_impl->square_to_bitboard_diag_a8h1(sq);
	}
	else {
		_black_pieces |= _bitboard_impl->square_to_bitboard(sq);
		_black_pieces_transpose |= _bitboard_impl->square_to_bitboard_transpose(sq);
		_black_pieces_diag_a1h8 |= _bitboard_impl->square_to_bitboard_diag_a1h8(sq);
		_black_pieces_diag_a8h1 |= _bitboard_impl->square_to_bitboard_diag_a8h1(sq);
	}
	
}

void PositionState::remove_piece_from_bitboards(Square sq, Color clr)
{
	assert(clr == WHITE || clr == BLACK);
	if (clr == WHITE) {
		_white_pieces ^= _bitboard_impl->square_to_bitboard(sq);
		_white_pieces_transpose ^= _bitboard_impl->square_to_bitboard_transpose(sq);
		_white_pieces_diag_a1h8 ^= _bitboard_impl->square_to_bitboard_diag_a1h8(sq);
		_white_pieces_diag_a8h1 ^= _bitboard_impl->square_to_bitboard_diag_a8h1(sq);
	}
	else {
		_black_pieces ^= _bitboard_impl->square_to_bitboard(sq);
		_black_pieces_transpose ^= _bitboard_impl->square_to_bitboard_transpose(sq);
		_black_pieces_diag_a1h8 ^= _bitboard_impl->square_to_bitboard_diag_a1h8(sq);
		_black_pieces_diag_a8h1 ^= _bitboard_impl->square_to_bitboard_diag_a8h1(sq);
	}
	
}
void PositionState::print_white_pieces() const
{
	std::cout << "White pieces:" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if ((_white_pieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_WHITE: std::cout << "P ";
						break;
					case KNIGHT_WHITE: std::cout << "N ";
						break;
					case BISHOP_WHITE: std::cout << "B ";
						break;
					case ROOK_WHITE: std::cout << "R ";
						break;
					case QUEEN_WHITE: std::cout << "Q ";
						break;
					case KING_WHITE: std::cout << "K ";
						break;
					default: std::cout << "X "; 
				}
			}
			else {
				std::cout << "E ";
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void PositionState::print_black_pieces() const
{
	std::cout << "Black pieces:" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if ((_black_pieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_BLACK: std::cout << "P ";
						break;
					case KNIGHT_BLACK: std::cout << "N ";
						break;
					case BISHOP_BLACK: std::cout << "B ";
						break;
					case ROOK_BLACK: std::cout << "R ";
						break;
					case QUEEN_BLACK: std::cout << "Q ";
						break;
					case KING_BLACK: std::cout << "K ";
						break;
					default: std::cout << "X ";
				}
			}
			else {
				std::cout << "E ";
			}

			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void PositionState::print_board() const
{
	std::cout << "Complete board" << std::endl; 
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			switch(_board[i][j]) {
				case PAWN_WHITE: std::cout << "PW ";
					break;
				case KNIGHT_WHITE: std::cout << "NW ";
					break;
				case BISHOP_WHITE: std::cout << "BW ";
					break;
				case ROOK_WHITE: std::cout << "RW ";
					break;
				case QUEEN_WHITE: std::cout << "QW ";
					break;
				case KING_WHITE: std::cout << "KW ";
					break;
				case PAWN_BLACK: std::cout << "PB ";
					break;
				case KNIGHT_BLACK: std::cout << "NB ";
					break;
				case BISHOP_BLACK: std::cout << "BB ";
					break;
				case ROOK_BLACK: std::cout << "RB ";
					break;
				case QUEEN_BLACK: std::cout << "QB ";
					break;
				case KING_BLACK: std::cout << "KB ";
					break;
				case ETY_SQUARE: std::cout << "ES ";
					break;
				default: std::cout << "XX ";
					break;
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}

void PositionState::print_possible_moves(Square from) const
{
	Piece promoted;
	if (_white_to_play) {
		promoted = QUEEN_WHITE;
	}
	else {
		promoted = QUEEN_BLACK;
	}
	std::cout << "Possible moves" << std::endl;
	for (int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			move_info move = {from, (Square) (i * 8 + j), promoted};
			if (move_is_legal(move)) {
				std::cout << "L ";
			}
			else {
				std::cout << "X ";
			}
			
			if (j == 7) {
				std::cout << std::endl;
			}
		}
	}
}
}
