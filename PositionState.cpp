#include "PositionState.h"
#include <iostream>

namespace pismo
{

PositionState::PositionState() : 
_white_to_play(true),
_king_under_attack(false),
_en_passant_file(-1),
_white_pieces(0), 
_black_pieces(0)
_white_left_castling(true);
_white_right_castling(true);
_black_left_castling(true);
_black_right_castling(true);
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

bool PositionState::move_is_legal(Square from, Square to) const
{
	Piece pfrom = board[from / 8][from % 8];
	Piece pto = board[to / 8][to % 8];
	if (pfrom <= KING_WHITE && whiteToPlay && pto == ETY_SQUARE)
		return true;
	else if (pfrom >= PAWN_BLACK && pfrom <= KING_BLACK && !whiteToPlay && pto == ETY_SQUARE)
		return true;
	else
		return false;
}

void PositionState::make_move(const move_info& move)
{
	move_type type;
	if (move_is_legal(move, type)) {
	switch(type) {
		case NORMAL_MOVE: 
			make_normal_move(move);
			break;
		case CASTLING:
			make_castling_move(move);
			break;
		case EN_PASSANT_MOVE:
			make_en_passant_move(move);
			break;
		case EN_PASSANT_CAPTURE:
			make_en_passant_capture(move);
			break;
		case PROMOTION:
			make_promotion_move(move);
			break;
	}
	
	if (white_to_play) {
		if (white_left_castling || white_right_castling) {
			if (_board[E1 / 8][E1 % 8] != KING_WHITE){
				white_left_castling = false;
				white_right_castling = false;
			}
			else if (_board[A1 / 8][A1 % 8] != ROOK_WHITE) {
				white_left_castling = false;
			}
			else if (_board[H1 / 8][H1 % 8] != ROOK_WHITE) {
				white_right_castling = false;
			}
		}
		// todo : check if black king is under attack
	}
	else {
		if (black_left_castling || black_right_castling) {
			if (_board[E8 / 8][E8 % 8] != KING_BLACK){
				black_left_castling = false;
				black_right_castling = false;
			}
			else if (_board[A8 / 8][A8 % 8] != ROOK_BLACK) {
				black_left_castling = false;
			}
			else if (_board[H8 / 8][H8 % 8] != ROOK_BLACK) {
				black_right_castling = false;
			}
		}
		// todo : check if white king is under attack
	}
	
	white_to_play = !white_to_play;
	}
}

void PositionState::make_normal_move(const move_info& move)
{
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (white_to_play) {			
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQAURE) {
			_black_pieces ^= (tmp << move.to);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQAURE) {
			_white_pieces ^= (tmp << move.to);
			--(_white_pieces_count[_board[move.to / 8][move.to % 8]]);
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = -1;				
}

void PositionState::make_castling_move(const move_info& move)
{
	if (white_to_play) {
		if (move.from < move.to) {
			_white_pieces ^= WHITE_RIGHT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQAURE;
			_board[move.to / 8][move.to % 8] = KING_WHITE;
			_board[move.to / 8][move.to % 8 + 1] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 - 1] = ROOK_WHITE;
		}	
		else {
			_white_pieces ^= WHITE_LEFT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQAURE;
			_board[move.to / 8][move.to % 8] = KING_WHITE;
			_board[move.to / 8][move.to % 8 - 2] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 + 1] = ROOK_WHITE;
		}
	}
	else {
		if (move.from < move.to) {
			_black_pieces ^= BLACK_RIGHT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQAURE;
			_board[move.to / 8][move.to % 8] = KING_BLACK;
			_board[move.to / 8][move.to % 8 + 1] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 - 1] = ROOK_BLACK;
		}	
		else {
			_black_pieces ^= BLACK_LEFT_CASTLING;	
			_board[move.from / 8][move.from % 8] = ETY_SQAURE;
			_board[move.to / 8][move.to % 8] = KING_BLACK;
			_board[move.to / 8][move.to % 8 - 2] = ETY_SQUARE;
			_board[move.to / 8][move.to % 8 + 1] = ROOK_BLACK;
		}

}

void PositionState::make_en_passant_move(const move_info& move)
{
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (white_to_play) {
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
	}
	else {
		_balck_pieces ^= (tmp << move.from);
		_balck_pieces |= (tmp << move.to);
	}	
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_en_passant_file = move.from % 8;					
}

void PositionState::make_en_passant_capture(const move_info& move)
{
	Bitboard tmp = 1;
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (white_to_play) {
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		_black_pieces ^= (tmp << (move.to - 8));
		--(_black_pieces_count[PAWN_BLACK % (PIECE_NB / 2)]);
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
	}
	else {
		_balck_pieces ^= (tmp << move.from);
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
	if (white_to_play) {
		_white_pieces ^= (tmp << move.from);
		_white_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQAURE) {
			_black_pieces ^= (tmp << move.to);
			--(_black_pieces_count[_board[move.to / 8][move.to % 8] % (PIECE_NB / 2)]);
		}
		--(_white_pieces_count[_board[move.from / 8][move.from % 8]]);
		++(_white_pieces_count[move.promoted]);
	}
	else {
		_black_pieces ^= (tmp << move.from);
		_black_pieces |= (tmp << move.to);
		if (_board[move.to / 8][move.to % 8] != ETY_SQAURE) {
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
