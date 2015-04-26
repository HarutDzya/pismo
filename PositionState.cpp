#include "PositionState.h"
#include <iostream>

namespace pismo
{

PositionState::PositionState(): 
_white_to_play(true),
_king_under_attack(false),
_en_passant_file(-1),
_white_pieces(0), 
_black_pieces(0),
_white_left_castling(true),
_white_right_castling(true),
_black_left_castling(true),
_black_right_castling(true)
{
	for(int i = 0; i < 8; ++i) {
		for(int j = 0; j < 8; ++j) {
			_board[i][j] = ETY_SQUARE;
		}
	}
	for(int i = 0; i < PIECE_NB / 2; ++i) {
		_white_pieces_count[i] = 0;
		_black_pieces_count[i] = 0;
	} 
}

void PositionState::set_piece(Square s, Piece p)
{
	_board[s / 8][s % 8] = p;
	Bitboard tmp = 1;
	if(p <= KING_WHITE) {
		_white_pieces |= (tmp << s);
	}
	else {
		_black_pieces |= (tmp << s);
	}
}

void PositionState::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	//TODO: check init position validity

	for(std::size_t i = 0; i < pieces.size(); ++i) {
		set_piece(pieces[i].first, pieces[i].second);
		if (pieces[i].second <= KING_WHITE) {
			++(_white_pieces_count[pieces[i].second]);
		}
		else {
			++(_black_pieces_count[pieces[i].second % (PIECE_NB / 2)]);
		}
	}
}

bool PositionState::move_is_legal(const move_info& move, move_type& type) const
{
	Piece pfrom = _board[move.from  / 8][move.from % 8];
	Piece pto = _board[move.to  / 8][move.to % 8];
	type = NORMAL_MOVE;
	Bitboard tmp = 1;
	if(pfrom == ETY_SQUARE) {
		return false;
	}

	if((pfrom == KING_WHITE && pto == ROOK_WHITE) || (pfrom == KING_BLACK && pto == ROOK_BLACK)) {
		type = CASTLING_MOVE;
		return castling_is_legal(move);
	}

	if(_white_to_play) {
		if((tmp << move.from & _black_pieces) || (tmp << move.to & _white_pieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_WHITE: 
					return pawn_move_is_legal(move, type);
				case KNIGHT_WHITE: 
					return knight_move_is_legal(move);
				case BISHOP_WHITE:
					return bishop_move_is_legal(move);
				case ROOK_WHITE:
					return rook_move_is_legal(move);
				case QUEEN_WHITE:
					return queen_move_is_legal(move);
				case KING_WHITE:
					return king_move_is_legal(move);
				default: 
					return false;
			}
		}
	}
	else {
		if((tmp << move.from & _white_pieces) || (tmp << move.to & _black_pieces)) {
			return false;
		}
		else {
			switch(pfrom) {
				case PAWN_BLACK: 
					return pawn_move_is_legal(move, type);
				case KNIGHT_BLACK: 
					return knight_move_is_legal(move);
				case BISHOP_BLACK:
					return bishop_move_is_legal(move);
				case ROOK_BLACK:
					return rook_move_is_legal(move);
				case QUEEN_BLACK:
					return queen_move_is_legal(move);
				case KING_BLACK:
					return king_move_is_legal(move);
				default: 
					return false;
			}
		}
	}

}

bool PositionState::pawn_move_is_legal(const move_info& move, move_type& type) const
{
	bool result = false; 
	Bitboard tmp = 1;
	Bitboard init;
	if (_white_to_play) {
		// Checks for single square movement of the pawn
		if (move.to - move.from == 8 && ((tmp << move.to) ^ _black_pieces)) {
			if (move.to >= A8) {
				type = PROMOTION_MOVE;
				result = true;
			}
			else {
				type = NORMAL_MOVE;
				result = true;
			}
		}
		// Checks for the move of pawn from game starting position
		else if (move.to - move.from == 16 && ((tmp << move.from) & PAWN_WHITE_INIT) &&
		!((tmp << (move.from + 8)) & (_white_pieces | _black_pieces)) && ((tmp << move.to) ^ _black_pieces)) {
			type = EN_PASSANT_MOVE;
			result = true;
		}
		// Checks for usual capture movement of the pawn, the last condition checks for edge capture
		else if ((move.to - move.from == 7 || move.to - move.from == 9) && (tmp << move.to & _black_pieces)
		&& (move.to / 8 - move.from / 8) == 1) {
			if (move.to >= A8) {
				type = PROMOTION_MOVE;
				result = true;
			}
			else {
				type = NORMAL_MOVE;
				result = true;
			}
		}
		else if (en_passant_capture_is_legal(move)) {
			type = EN_PASSANT_CAPTURE;
			result = true;
		}
	}
	else {
		// Checks for single square movement of the pawn
		if (move.from - move.to == 8 && ((tmp << move.to) ^ _white_pieces)) {
			if (move.to <= H1) {
				type = PROMOTION_MOVE;
				result = true;
			}
			else {
				type = NORMAL_MOVE;
				result = true;
			}
		}
		// Checks for the move of pawn from game starting position
		else if (move.from - move.to == 16 && ((tmp << move.from) & PAWN_BLACK_INIT) &&
		!((tmp << (move.from - 8)) & (_white_pieces | _black_pieces)) && ((tmp << move.to) ^ _white_pieces)) {
			type = EN_PASSANT_MOVE;
			result = true;
		}
		// Checks for usual capture movement of the pawn, the last condition checks for edge capture
		else if ((move.from - move.to == 7 || move.from - move.to == 9) && (tmp << move.to & _white_pieces)
		&& (move.from / 8 - move.to / 8) == 1) {
			if (move.to <= H1) {
				type = PROMOTION_MOVE;
				result = true;
			}
			else {
				type = NORMAL_MOVE;
				result = true;
			}
		}
		else if (en_passant_capture_is_legal(move)) {
			type = EN_PASSANT_CAPTURE;
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
	//TODO: add assertion to ensure there is an oponent's pawn in en_passant_file (square)	
		if (_white_to_play) {
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
	return true;
}

bool PositionState::bishop_move_is_legal(const move_info& move) const
{
	return true;
}

bool PositionState::rook_move_is_legal(const move_info& move) const
{
	return true;
}

bool PositionState::queen_move_is_legal(const move_info& move) const
{
	return true;
}

bool PositionState::king_move_is_legal(const move_info& move) const
{
	return true;
}

bool PositionState::castling_is_legal(const move_info& move) const
{
	return true;
}

bool PositionState::make_move(const move_info& move)
{
	move_type type;
	if (move_is_legal(move, type)) {
	switch(type) {
		case NORMAL_MOVE: 
			make_normal_move(move);
			break;
		case CASTLING_MOVE:
			make_castling_move(move);
			break;
		case EN_PASSANT_MOVE:
			make_en_passant_move(move);
			break;
		case EN_PASSANT_CAPTURE:
			make_en_passant_capture(move);
			break;
		case PROMOTION_MOVE:
			make_promotion_move(move);
			break;
	}
	
	update_castling_variables();
	// todo : check if opponents king is under attack and update the variable
	// accordingly
	
	_white_to_play = !_white_to_play;
	return true;
	}
	return false;
}

void PositionState::make_normal_move(const move_info& move)
{
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {			
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			_black_pieces ^= (tmp << move.to);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			_white_pieces ^= (tmp << move.to);
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
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
	}	
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = move.from % 8;					
}

void PositionState::make_en_passant_capture(const move_info& move)
{
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		_black_pieces ^= (tmp << (move.to - 8));
		--(_black_pieces_count[PAWN_BLACK % (PIECE_NB / 2)]);
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
		_white_pieces ^= (tmp << (move.to + 8));
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
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			_black_pieces ^= (tmp << move.to);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
		--(_white_pieces_count[_board[move.from / 8][move.from % 8]]);
		++(_white_pieces_count[move.promoted]);
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQUARE) {
			_white_pieces ^= (tmp << move.to);
			--(_white_pieces_count[_board[move.to / 8][move.to % 8]]);
		}
		--(_black_pieces_count[_board[move.from / 8][move.from % 8] % (PIECE_NB / 2)]);
		++(_black_pieces_count[move.promoted % (PIECE_NB / 2)]);
		
		}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = move.promoted;
	_en_passant_file = -1;
}

void PositionState::update_castling_variables()
{
	if (_white_to_play) {
		if (_white_left_castling || _white_right_castling) {
			if (_board[E1 / 8][E1 % 8] != KING_WHITE){
				_white_left_castling = false;
				_white_right_castling = false;
			}
			else if (_board[A1 / 8][A1 % 8] != ROOK_WHITE) {
				_white_left_castling = false;
			}
			else if (_board[H1 / 8][H1 % 8] != ROOK_WHITE) {
				_white_right_castling = false;
			}
		}
	}
	else {
		if (_black_left_castling || _black_right_castling) {
			if (_board[E8 / 8][E8 % 8] != KING_BLACK){
				_black_left_castling = false;
				_black_right_castling = false;
			}
			else if (_board[A8 / 8][A8 % 8] != ROOK_BLACK) {
				_black_left_castling = false;
			}
			else if (_board[H8 / 8][H8 % 8] != ROOK_BLACK) {
				_black_right_castling = false;
			}
		}
	}
}
	
void PositionState::print_white_pieces() const
{
	std::cout << "White pieces:" << std::endl;
	for(int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			if ((_white_pieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_WHITE: std::cout << "P ";
						break;
					case KNIGHT_WHITE: std::cout << "K ";
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
	for(int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			if ((_black_pieces >> (i * 8 + j)) & 1) {	
				switch(_board[i][j]) {
					case PAWN_BLACK: std::cout << "P ";
						break;
					case KNIGHT_BLACK: std::cout << "K ";
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
	for(int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			switch(_board[i][j]) {
				case PAWN_WHITE: std::cout << "PW ";
					break;
				case KNIGHT_WHITE: std::cout << "KW ";
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
				case KNIGHT_BLACK: std::cout << "KB ";
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
}
