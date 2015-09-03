#include "PositionState.h"
#include "BitboardImpl.h"
#include "ZobKeyImpl.h"
#include "PieceSquareTable.h"
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
_zob_key_impl(new ZobKeyImpl()),
_white_to_play(true),
_en_passant_file(-1),
_white_king_position(E1),
_black_king_position(E8),
_white_left_castling(false),
_white_right_castling(false),
_black_left_castling(false),
_black_right_castling(false),
_is_middle_game(true),
_zob_key(0),
_material_zob_key(0),
_pst_value(0),
_move_stack(),
_halfmove_clock(0),
_fullmove_count(1)
{
	for (unsigned int i = 0; i < 8; ++i) {
		for (unsigned int j = 0; j < 8; ++j) {
			_board[i][j] = ETY_SQUARE;
		}
	}
	for (unsigned int i = 0; i < PIECE_NB; ++i) {
		_piece_count[i] = 0;
	} 
	
	for (unsigned int piece = PAWN_WHITE; piece < PIECE_NB; ++piece) {
		_material_zob_key ^= _zob_key_impl->get_material_key((Piece)(piece), 0);
	}
}

PositionState::~PositionState()
{
	delete _bitboard_impl;
	delete _zob_key_impl;
}

void PositionState::set_piece(Square s, Piece p)
{
	_board[s / 8][s % 8] = p;
	if (p <= KING_WHITE) {
		if (p == KING_WHITE) {
			_white_king_position = s;
		}
		add_piece_to_bitboards(s, WHITE);
	}
	else {
		if (p == KING_BLACK) {
			_black_king_position = s;
		}
		add_piece_to_bitboards(s, BLACK);
	}
	++_piece_count[p];
	_pst_value += calculate_pst_value(p, s);
	_zob_key ^=  _zob_key_impl->get_piece_at_square_key(p, s);

	_material_zob_key ^= _zob_key_impl->get_material_key(p, _piece_count[p]);
}

void PositionState::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	if (init_position_is_valid(pieces)) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			set_piece(pieces[i].first, pieces[i].second);
		}
	
		_white_left_castling = true;
		_white_right_castling = true;
		_black_left_castling = true;
		_black_right_castling = true;
		_zob_key ^= _zob_key_impl->get_white_left_castling_key();
		_zob_key ^= _zob_key_impl->get_white_right_castling_key();
		_zob_key ^= _zob_key_impl->get_black_left_castling_key();
		_zob_key ^= _zob_key_impl->get_black_right_castling_key();

		update_game_status();
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
	unsigned int pieces_count[PIECE_NB] = {};
	unsigned int white_pieces_sum = 0;
	unsigned int black_pieces_sum = 0;
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

	unsigned int white_promoted_pieces_sum = 0;
	unsigned int black_promoted_pieces_sum = 0;

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
				white_promoted_pieces_sum += ((int) pieces_count[piece] - 1) > 0 ? pieces_count[piece] - 1 : 0;
				break;
			case QUEEN_BLACK:
				black_promoted_pieces_sum += ((int) pieces_count[piece] - 1) > 0 ? pieces_count[piece] - 1 : 0;
				break;
			default:
				if (piece < KING_WHITE) {
					white_promoted_pieces_sum += ((int) pieces_count[piece] - 2) > 0 ? pieces_count[piece] - 2 : 0;
				}
				else {
				 	black_promoted_pieces_sum += ((int) pieces_count[piece] - 2) > 0 ? pieces_count[piece] - 2 : 0;
				}
		}
	}
	
	if ((white_promoted_pieces_sum > 8 - pieces_count[PAWN_WHITE]) || (black_promoted_pieces_sum > 8 - pieces_count[PAWN_BLACK])) {
		return false;
	}
	
	return true; 
}

void PositionState::init_position_FEN(const std::string& fen)
{
	unsigned int char_count = 0;
	init_material_FEN(fen, char_count);
	++char_count;
	init_right_to_play_FEN(fen, char_count);
	++char_count;
	init_castling_rights_FEN(fen, char_count);
	++char_count;
	init_en_passant_file_FEN(fen, char_count);
	++char_count;
	init_move_count_FEN(fen, char_count);
	
	update_game_status();
	assert(char_count == fen.size());
}


void PositionState::init_material_FEN(const std::string& fen, unsigned int& char_count)
{
	std::vector<std::pair<Square, Piece> > pieces;
	unsigned int rank = 7;
	unsigned int file = 0;
	while(fen[char_count] != ' ') {
		switch(fen[char_count]) {
			case 'P':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), PAWN_WHITE));
				++file;
				break;
			case 'N':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KNIGHT_WHITE));
				++file;
				break;
			case 'B':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), BISHOP_WHITE));
				++file;
				break;
			case 'R':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), ROOK_WHITE));
				++file;
				break;
			case 'Q':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), QUEEN_WHITE));
				++file;
				break;
			case 'K':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KING_WHITE));
				++file;
				break;
			case 'p':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), PAWN_BLACK));
				++file;
				break;
			case 'n':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KNIGHT_BLACK));
				++file;
				break;
			case 'b':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), BISHOP_BLACK));
				++file;
				break;
			case 'r':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), ROOK_BLACK));
				++file;
				break;
			case 'q':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), QUEEN_BLACK));
				++file;
				break;
			case 'k':
				pieces.push_back(std::pair<Square, Piece>((Square) (rank * 8 + file), KING_BLACK));
				++file;
				break;
			case '1':
				file += 1;
				break;
			case '2':
				file += 2;
			 	break;
			case '3':
				file += 3;
				break;
			case '4':
				file += 4;
				break;
			case '5':
				file += 5;
				break;
			case '6':
				file += 6;
				break;
			case '7':
				file += 7;
				break;
			case '8':
				// file is updated directly in case '/'
				break;
			case '/':
				--rank;
				file = 0;
				break;
			default:
				assert(false);
		}
		
		++char_count;
	}

	if (init_position_is_valid(pieces)) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			set_piece(pieces[i].first, pieces[i].second);
		}
	}
}

void PositionState::init_right_to_play_FEN(const std::string& fen, unsigned int& char_count)
{
	if(fen[char_count++] == 'w') {
		_white_to_play = true;
	}
	else {
		_white_to_play = false;
		_zob_key ^= _zob_key_impl->get_if_black_to_play_key();
	}
}


void PositionState::init_castling_rights_FEN(const std::string& fen, unsigned int& char_count)
{
	while(fen[char_count] != ' ') {
		switch(fen[char_count]) {
			case 'K':
				_white_right_castling = true;
				_zob_key ^= _zob_key_impl->get_white_right_castling_key();
				break;
			case 'Q':
				_white_left_castling = true;
				_zob_key ^= _zob_key_impl->get_white_left_castling_key();
				break;
			case 'k':
				_black_right_castling = true;
				_zob_key ^= _zob_key_impl->get_black_right_castling_key();
				break;
			case 'q':
				_black_left_castling = true;
				_zob_key ^= _zob_key_impl->get_black_left_castling_key();
				break;
			case '-':
				break;
			default:
				assert(false);
		}
		++char_count;
	}
}

void PositionState::init_en_passant_file_FEN(const std::string& fen, unsigned int& char_count)
{
	if(fen[char_count] == '-') {
		_en_passant_file = -1;
		++char_count;
	}
	else {	
		_en_passant_file = fen[char_count] - 'a';
		++char_count;
		assert(_en_passant_file >= 0 && _en_passant_file < 8);
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
	}
}

void PositionState::init_move_count_FEN(const std::string& fen, unsigned int& char_count)
{
	_halfmove_clock = std::atoi(&fen[char_count]);
	while(fen[char_count] != ' ') {
		++char_count;
	}
	_fullmove_count = std::atoi(&fen[char_count]);
	while(char_count != fen.size()) {
		++char_count;
	}
}

bool PositionState::move_is_legal(const move_info& move) const
{
	assert(move.from != move.to);
	Piece pfrom = _board[move.from  / 8][move.from % 8];
	if(pfrom == ETY_SQUARE) {
		return false;
	}

	bool result = false;
	if(_white_to_play) {
		if((_bitboard_impl->square_to_bitboard(move.from) & _black_pieces) || (_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			return false;
		}
		else {
			bool is_en_passant_capture = false;
			switch(pfrom) {
				case PAWN_WHITE: 
					result = pawn_move_is_legal(move, is_en_passant_capture);
					break;
				case KNIGHT_WHITE:
					result = knight_move_is_legal(move);
					break;
				case BISHOP_WHITE:
					result = bishop_move_is_legal(move);
					break;
				case ROOK_WHITE:
					result = rook_move_is_legal(move);
					break;
				case QUEEN_WHITE:
					result = queen_move_is_legal(move);
					break;
				case KING_WHITE:
					if (std::abs(move.from % 8 - move.to % 8) > 1) {
						return castling_is_legal(move);
					}
					else { 
						result = king_move_is_legal(move);
					}
					break;
				default: 
					assert(pfrom >= PAWN_WHITE && pfrom <= KING_WHITE);
			}
	
			if (result) {
				Piece captured_piece;
				make_lazy_move(move, is_en_passant_capture, captured_piece);
				result = !(_bitboard_impl->square_to_bitboard(_white_king_position) & squares_under_attack(WHITE));
				undo_lazy_move(move, is_en_passant_capture, captured_piece);
			}
		}
	}
	else {
		if((_bitboard_impl->square_to_bitboard(move.from) & _white_pieces) || (_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			return false;
		}
		else {
			bool is_en_passant_capture = false;
			switch(pfrom) {
				case PAWN_BLACK: 
					result = pawn_move_is_legal(move, is_en_passant_capture);
					break;
				case KNIGHT_BLACK: 
					result = knight_move_is_legal(move);
					break;
				case BISHOP_BLACK:
					result = bishop_move_is_legal(move);
					break;
				case ROOK_BLACK:
					result = rook_move_is_legal(move);
					break;
				case QUEEN_BLACK:
					result = queen_move_is_legal(move);
					break;
				case KING_BLACK:
					if (std::abs(move.from % 8 - move.to % 8) > 1) { 
						return castling_is_legal(move);
					}
					else { 
						result = king_move_is_legal(move);
					}
					break;
				default: 
					return false;
			}
		
			if (result) {
				Piece captured_piece;
				make_lazy_move(move, is_en_passant_capture, captured_piece);
				result = !(_bitboard_impl->square_to_bitboard(_black_king_position) & squares_under_attack(BLACK));
				undo_lazy_move(move, is_en_passant_capture, captured_piece);
			}
		}
	}

	return result;
}

bool PositionState::pawn_move_is_legal(const move_info& move, bool& is_en_passant_capture) const
{
	is_en_passant_capture = false;
	if (_white_to_play) {
		assert(move.from > H1);
		// Checks for single square movement of the pawn
		if (move.to - move.from == 8 && !(_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			return true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.to - move.from == 16 && (_bitboard_impl->square_to_bitboard(move.from) & PAWN_WHITE_INIT) &&
		!(_bitboard_impl->square_to_bitboard((Square)(move.from + 8)) & (_white_pieces | _black_pieces)) && !(_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			return true;
		}
		// Checks for usual capture movement of the pawn
		else if ((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_pawn_white_attacking_moves(move.from)) && (_bitboard_impl->square_to_bitboard(move.to) & _black_pieces)) {
			if (move.to >= A8) {
				assert(move.promoted > PAWN_WHITE && move.promoted < KING_WHITE);
			}
			return true;
		}
		else {
			is_en_passant_capture = true;
			return en_passant_capture_is_legal(move);
		}
	}
	else {
		// Checks for single square movement of the pawn
		assert(move.from < A8);
		if (move.from - move.to == 8 && !(_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			return true;
		}
		// Checks for the move of pawn from game starting position
		else if (move.from - move.to == 16 && (_bitboard_impl->square_to_bitboard(move.from) & PAWN_BLACK_INIT) &&
		!(_bitboard_impl->square_to_bitboard((Square)(move.from - 8)) & (_white_pieces | _black_pieces)) && !(_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			return true;
		}
		// Checks for usual capture movement of the pawn
		else if ((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_pawn_black_attacking_moves(move.from)) && (_bitboard_impl->square_to_bitboard(move.to) & _white_pieces)) {
			if (move.to <= H1) {
				assert(move.promoted > PAWN_BLACK && move.promoted < KING_BLACK);
			}
			return true;
		}
		else {
			is_en_passant_capture = true;
			return  en_passant_capture_is_legal(move);
		}
	}
	return false;
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
			else if (((move.from == 4 * 8 + _en_passant_file + 1) || (move.from == 4 * 8 + _en_passant_file - 1))
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
			else if (((move.from == 3 * 8 + _en_passant_file + 1) || (move.from == 3 * 8 + _en_passant_file - 1))
			&& move.to == 2 * 8 + _en_passant_file) {
				return true;
			}
		}
 	}
	
	return false;
}

bool PositionState::knight_move_is_legal(const move_info& move) const
{
	if (_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_knight_moves(move.from)) {
		return true;
	}
	return false;
}

bool PositionState::bishop_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _bitboard_impl->get_legal_diag_a1h8_moves(move.from, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8))
	|| (_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _bitboard_impl->get_legal_diag_a8h1_moves(move.from, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1))) {
		return true;
	}
	return false;
}

bool PositionState::rook_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_rank_moves(move.from, _white_pieces | _black_pieces))
	|| (_bitboard_impl->square_to_bitboard_transpose(move.to) & _bitboard_impl->get_legal_file_moves(move.from, _white_pieces_transpose | _black_pieces_transpose))) {
		return true;
	}
	return false;
}

bool PositionState::queen_move_is_legal(const move_info& move) const
{
	if((_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_rank_moves(move.from, _white_pieces | _black_pieces))
	|| (_bitboard_impl->square_to_bitboard_transpose(move.to) & _bitboard_impl->get_legal_file_moves(move.from, _white_pieces_transpose | _black_pieces_transpose)) 
	|| (_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _bitboard_impl->get_legal_diag_a1h8_moves(move.from, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8))
	|| (_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _bitboard_impl->get_legal_diag_a8h1_moves(move.from, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1))) {
		return true;
	}
	return false;
}

bool PositionState::king_move_is_legal(const move_info& move) const
{
	if (_bitboard_impl->square_to_bitboard(move.to) & _bitboard_impl->get_legal_king_moves(move.from)) {
		return true;
	}
	return false;
}

bool PositionState::castling_is_legal(const move_info& move) const
{
	if (_white_to_play) {
		if (move.from == E1 && move.to == C1) {
			if (_white_left_castling && !(WHITE_LEFT_CASTLING_ETY_SQUARES & (_white_pieces | _black_pieces)) && !(WHITE_LEFT_CASTLING_KING_SQUARES & squares_under_attack(WHITE))) {
					return true;
				}
		}
		else if (move.from == E1 && move.to == G1) {
				if (_white_right_castling && !(WHITE_RIGHT_CASTLING_ETY_SQUARES & (_white_pieces | _black_pieces)) && !(WHITE_RIGHT_CASTLING_KING_SQUARES & squares_under_attack(WHITE))) {
					return true;
				}
		}
		
		return false;
	}
	else {
		if (move.from == E8 && move.to == C8) {
			if (_black_left_castling && !(BLACK_LEFT_CASTLING_ETY_SQUARES & (_white_pieces | _black_pieces)) && !(BLACK_LEFT_CASTLING_KING_SQUARES & squares_under_attack(BLACK))) {
					return true;
				}
		}
		else if (move.from == E8 && move.to == G8) {
				if (_black_right_castling && !(BLACK_RIGHT_CASTLING_ETY_SQUARES & (_white_pieces | _black_pieces)) && !(BLACK_RIGHT_CASTLING_KING_SQUARES & squares_under_attack(BLACK))) {
					return true;
				}
		}
		
		return false;
	}
}

// Returns a bitboard with the bit set at the positions where the 
// attacked_color pieces are under attack by opponent
Bitboard PositionState::squares_under_attack(Color attacked_color) const
{
	Bitboard attacked_bitboard = 0;
	Bitboard attacked_bitboard_transpose = 0;
	Bitboard attacked_bitboard_diag_a1h8 = 0;
	Bitboard attacked_bitboard_diag_a8h1 = 0;
	if(attacked_color == BLACK) {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_WHITE:
					attacked_bitboard |= _bitboard_impl->get_legal_pawn_white_attacking_moves((Square) sq);
					break;
				case KNIGHT_WHITE:
					attacked_bitboard |= _bitboard_impl->get_legal_knight_moves((Square) sq);
					break;
				case BISHOP_WHITE:
					attacked_bitboard_diag_a1h8 |= _bitboard_impl->get_legal_diag_a1h8_moves((Square) sq, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
					attacked_bitboard_diag_a8h1 |= _bitboard_impl->get_legal_diag_a8h1_moves((Square) sq, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
					break;
				case ROOK_WHITE:
					attacked_bitboard |= _bitboard_impl->get_legal_rank_moves((Square) sq, _white_pieces | _black_pieces);
					attacked_bitboard_transpose |= _bitboard_impl->get_legal_file_moves((Square) sq, _white_pieces_transpose | _black_pieces_transpose);
					break;
				case QUEEN_WHITE:
					attacked_bitboard |= _bitboard_impl->get_legal_rank_moves((Square) sq, _white_pieces | _black_pieces);
					attacked_bitboard_transpose |= _bitboard_impl->get_legal_file_moves((Square) sq, _white_pieces_transpose | _black_pieces_transpose);
					attacked_bitboard_diag_a1h8 |= _bitboard_impl->get_legal_diag_a1h8_moves((Square) sq, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
					attacked_bitboard_diag_a8h1 |= _bitboard_impl->get_legal_diag_a8h1_moves((Square) sq, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
					break;
				case KING_WHITE:	
					attacked_bitboard |= _bitboard_impl->get_legal_king_moves((Square) sq);
					break;
				default:
					break;
			}
		}
	}
	else {
		for (unsigned int sq = A1; sq <= H8; ++sq) {
			switch(_board[sq / 8][sq % 8]) {
				case PAWN_BLACK:
					attacked_bitboard |= _bitboard_impl->get_legal_pawn_black_attacking_moves((Square) sq);
					break;
				case KNIGHT_BLACK:
					attacked_bitboard |= _bitboard_impl->get_legal_knight_moves((Square) sq);
					break;
				case BISHOP_BLACK:
					attacked_bitboard_diag_a1h8 |= _bitboard_impl->get_legal_diag_a1h8_moves((Square) sq, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
					attacked_bitboard_diag_a8h1 |= _bitboard_impl->get_legal_diag_a8h1_moves((Square) sq, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
					break;
				case ROOK_BLACK:
					attacked_bitboard |= _bitboard_impl->get_legal_rank_moves((Square) sq, _white_pieces | _black_pieces);
					attacked_bitboard_transpose |= _bitboard_impl->get_legal_file_moves((Square) sq, _white_pieces_transpose | _black_pieces_transpose);
					break;
				case QUEEN_BLACK:
					attacked_bitboard |= _bitboard_impl->get_legal_rank_moves((Square) sq, _white_pieces | _black_pieces);
					attacked_bitboard_transpose |= _bitboard_impl->get_legal_file_moves((Square) sq, _white_pieces_transpose | _black_pieces_transpose);
					attacked_bitboard_diag_a1h8 |= _bitboard_impl->get_legal_diag_a1h8_moves((Square) sq, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
					attacked_bitboard_diag_a8h1 |= _bitboard_impl->get_legal_diag_a8h1_moves((Square) sq, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
					break;
				case KING_BLACK:	
					attacked_bitboard |= _bitboard_impl->get_legal_king_moves((Square) sq);
					break;
				default:
					break;
			}
		}
	}	

	attacked_bitboard |= (_bitboard_impl->bitboard_transpose_to_bitboard(attacked_bitboard_transpose) | _bitboard_impl->bitboard_diag_a1h8_to_bitboard(attacked_bitboard_diag_a1h8) | _bitboard_impl->bitboard_diag_a8h1_to_bitboard(attacked_bitboard_diag_a8h1));

	if (attacked_color == BLACK) {
		return attacked_bitboard & (~_white_pieces);
	}
	else {
		return attacked_bitboard & (~_black_pieces);
	}
}

// Makes move by only updating the occupation bitboards and _board array
// Castling move and also promotion are not considered here,
// therefore pawns can appear at the first and last ranks due to this move
// This should be always used in conjunction with undo_lazy_move 
void PositionState::make_lazy_move(const move_info& move, bool is_en_passant_capture, Piece& captured_piece) const
{
	PositionState * non_const_this = const_cast<PositionState *>(this);
	captured_piece = _board[move.to / 8][move.to % 8];
	if (_white_to_play) {
		if (is_en_passant_capture) {
			non_const_this->remove_piece_from_bitboards((Square) (move.to - 8), BLACK);
			(non_const_this->_board)[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
			}
		else {
			if(captured_piece != ETY_SQUARE) {
				non_const_this->remove_piece_from_bitboards(move.to, BLACK);
			}
		}
		non_const_this->remove_piece_from_bitboards(move.from, WHITE);
		non_const_this->add_piece_to_bitboards(move.to, WHITE);
	}
	else {
		if (is_en_passant_capture) {
			non_const_this->remove_piece_from_bitboards((Square) (move.to + 8), WHITE);
			(non_const_this->_board)[move.to / 8 + 1][move.to % 8] = ETY_SQUARE;
			}
		else {
			if(captured_piece != ETY_SQUARE) {
				non_const_this->remove_piece_from_bitboards(move.to, WHITE);
			}
		}
		non_const_this->remove_piece_from_bitboards(move.from, BLACK);
		non_const_this->add_piece_to_bitboards(move.to, BLACK);
	}
	Piece pfrom = _board[move.from / 8][move.from % 8]; 
	(non_const_this->_board)[move.to / 8][move.to % 8] = pfrom;	
	(non_const_this->_board)[move.from / 8][move.from % 8] = ETY_SQUARE;
	if (pfrom == KING_WHITE) {
		non_const_this->_white_king_position = move.to;
	}
	else if (pfrom == KING_BLACK) {
		non_const_this->_black_king_position = move.to;
	}

}

// Reverses the lazy move done by make_lazy_move
void PositionState::undo_lazy_move(const move_info& move, bool is_en_passant_capture, Piece captured_piece) const
{
	PositionState * non_const_this = const_cast<PositionState *>(this);
	if (_white_to_play) {
		if (is_en_passant_capture) {
			non_const_this->add_piece_to_bitboards((Square) (move.to - 8), BLACK);
			(non_const_this->_board)[move.to / 8 - 1][move.to % 8] = PAWN_BLACK;
			}
		else {
			if(captured_piece != ETY_SQUARE) {
				non_const_this->add_piece_to_bitboards(move.to, BLACK);
			}
		}
		non_const_this->add_piece_to_bitboards(move.from, WHITE);
		non_const_this->remove_piece_from_bitboards(move.to, WHITE);
	}
	else {
		if (is_en_passant_capture) {
			non_const_this->add_piece_to_bitboards((Square) (move.to + 8), WHITE);
			(non_const_this->_board)[move.to / 8 + 1][move.to % 8] = PAWN_WHITE;
			}
		else {
			if(captured_piece != ETY_SQUARE) {
				non_const_this->add_piece_to_bitboards(move.to, WHITE);
			}
		}
		non_const_this->add_piece_to_bitboards(move.from, BLACK);
		non_const_this->remove_piece_from_bitboards(move.to, BLACK);
	}
	Piece pto = _board[move.to / 8][move.to % 8];
	(non_const_this->_board)[move.from / 8][move.from % 8] = pto;
	(non_const_this->_board)[move.to / 8][move.to % 8] = captured_piece;
	if (pto == KING_WHITE) {
		non_const_this->_white_king_position = move.from;
	}
	else if (pto == KING_BLACK) {
		non_const_this->_black_king_position = move.from;
	}
}

void PositionState::update_direct_check_array()
{
	if (_white_to_play) {
		_direct_check[KING_WHITE] = 0;
		_direct_check[KNIGHT_WHITE] = _bitboard_impl->get_legal_knight_moves(_black_king_position);
		_direct_check[PAWN_WHITE] = _bitboard_impl->get_pawn_white_checking_pos(_black_king_position);
		Bitboard rank_attack = _bitboard_impl->get_legal_rank_moves(_black_king_position, _white_pieces | _black_pieces);
		Bitboard file_attack = _bitboard_impl->bitboard_transpose_to_bitboard(_bitboard_impl->get_legal_file_moves(_black_king_position, _white_pieces_transpose | _black_pieces_transpose));
		Bitboard diag_a1h8_attack = _bitboard_impl->bitboard_diag_a1h8_to_bitboard(_bitboard_impl->get_legal_diag_a1h8_moves(_black_king_position, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8));
		Bitboard diag_a8h1_attack = _bitboard_impl->bitboard_diag_a8h1_to_bitboard(_bitboard_impl->get_legal_diag_a8h1_moves(_black_king_position, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1));
		_direct_check[ROOK_WHITE] = rank_attack | file_attack;
		_direct_check[BISHOP_WHITE] = diag_a1h8_attack | diag_a8h1_attack;
		_direct_check[QUEEN_WHITE] = rank_attack | file_attack | diag_a1h8_attack | diag_a8h1_attack;
	}
	else {
		_direct_check[KING_BLACK] = 0;
		_direct_check[KNIGHT_BLACK] = _bitboard_impl->get_legal_knight_moves(_white_king_position);
		_direct_check[PAWN_BLACK] = _bitboard_impl->get_pawn_black_checking_pos(_white_king_position);
		Bitboard rank_attack = _bitboard_impl->get_legal_rank_moves(_white_king_position, _white_pieces | _black_pieces);
		Bitboard file_attack = _bitboard_impl->bitboard_transpose_to_bitboard(_bitboard_impl->get_legal_file_moves(_white_king_position, _white_pieces_transpose | _black_pieces_transpose));
		Bitboard diag_a1h8_attack = _bitboard_impl->bitboard_diag_a1h8_to_bitboard(_bitboard_impl->get_legal_diag_a1h8_moves(_white_king_position, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8));
		Bitboard diag_a8h1_attack = _bitboard_impl->bitboard_diag_a8h1_to_bitboard(_bitboard_impl->get_legal_diag_a8h1_moves(_white_king_position, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1));
		_direct_check[ROOK_BLACK] = rank_attack | file_attack;
		_direct_check[BISHOP_BLACK] = diag_a1h8_attack | diag_a8h1_attack;
		_direct_check[QUEEN_BLACK] = rank_attack | file_attack | diag_a1h8_attack | diag_a8h1_attack;
	}
}

void PositionState::update_discovered_checks()
{
	if (_white_to_play) {
		_pin_info.rank_pin = _bitboard_impl->get_rank_pin_pos(_black_king_position, _white_pieces | _black_pieces);
		_pin_info.file_pin = _bitboard_impl->get_file_pin_pos(_black_king_position, _white_pieces_transpose | _black_pieces_transpose);
		_pin_info.diag_a1h8_pin = _bitboard_impl->get_diag_a1h8_pin_pos(_black_king_position, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
		_pin_info.diag_a8h1_pin = _bitboard_impl->get_diag_a8h1_pin_pos(_black_king_position, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
		update_pin_ray_status(_pin_info.rank_pin, WHITE, false);
		update_pin_ray_status(_pin_info.file_pin, WHITE, false);
		update_pin_ray_status(_pin_info.diag_a1h8_pin, WHITE, true);
		update_pin_ray_status(_pin_info.diag_a8h1_pin, WHITE, true);
	}
	else {
		_pin_info.rank_pin = _bitboard_impl->get_rank_pin_pos(_white_king_position, _white_pieces | _black_pieces);
		_pin_info.file_pin = _bitboard_impl->get_file_pin_pos(_white_king_position, _white_pieces_transpose | _black_pieces_transpose);
		_pin_info.diag_a1h8_pin = _bitboard_impl->get_diag_a1h8_pin_pos(_white_king_position, _white_pieces_diag_a1h8 | _black_pieces_diag_a1h8);
		_pin_info.diag_a8h1_pin = _bitboard_impl->get_diag_a8h1_pin_pos(_white_king_position, _white_pieces_diag_a8h1 | _black_pieces_diag_a8h1);
		update_pin_ray_status(_pin_info.rank_pin, BLACK, false);
		update_pin_ray_status(_pin_info.file_pin, BLACK, false);
		update_pin_ray_status(_pin_info.diag_a1h8_pin, BLACK, true);
		update_pin_ray_status(_pin_info.diag_a8h1_pin, BLACK, true);
	}
}

void PositionState::update_pin_ray_status(pin_info& pin, Color clr, bool is_diag_ray) const
{
	Piece slide_piece;
	Piece queen_piece;
	if (clr == WHITE) {
		if (is_diag_ray) {
			slide_piece = BISHOP_WHITE;
		}
		else {
			slide_piece = ROOK_WHITE;
		}
		queen_piece = QUEEN_WHITE;
	}
	else {
		if (is_diag_ray) {
			slide_piece = BISHOP_BLACK;
		}
		else {
			slide_piece = ROOK_BLACK;
		}
		queen_piece = QUEEN_BLACK;
	}
	if (pin.small_square_slide != INVALID_SQUARE) {
		if (_board[pin.small_square_slide / 8][pin.small_square_slide % 8] != slide_piece && _board[pin.small_square_slide / 8][pin.small_square_slide % 8] != queen_piece) {
			pin.small_square_slide = INVALID_SQUARE;
			pin.small_pin_pos = 0;
		}
	}
	if (pin.big_square_slide != INVALID_SQUARE) {
		if (_board[pin.big_square_slide / 8][pin.big_square_slide % 8] != slide_piece && _board[pin.big_square_slide / 8][pin.big_square_slide % 8] != queen_piece) {
			pin.big_square_slide = INVALID_SQUARE;
			pin.big_pin_pos = 0;
		}
	}
}

bool PositionState::move_checks_opponent_king(const move_info& move) const
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_bitboard_impl->square_to_bitboard(move.to) & _direct_check[pfrom]) {
		return true;
	}	   
	
	if (move_opens_discovered_check(move)) {
		return true;
	}

	if (move.promoted != ETY_SQUARE) {
		if (_bitboard_impl->square_to_bitboard(move.to) & _direct_check[move.promoted]) {
			return true;
		}
	}
		
	if ((pfrom == KING_WHITE || pfrom == KING_BLACK) && std::abs(move.from % 8 - move.to % 8) > 1) {
		if (castling_checks_opponent_king(move)) {
			return true;
		}
	}

	return false;
}

bool PositionState::move_opens_discovered_check(const move_info& move) const
{
	if (_pin_info.rank_pin.small_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard(move.from) & _pin_info.rank_pin.small_pin_pos) && !(_bitboard_impl->square_to_bitboard(move.to) & _pin_info.rank_pin.small_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.rank_pin.big_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard(move.from) & _pin_info.rank_pin.big_pin_pos) && !(_bitboard_impl->square_to_bitboard(move.to) & _pin_info.rank_pin.big_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.file_pin.small_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_transpose(move.from) & _pin_info.file_pin.small_pin_pos) && !(_bitboard_impl->square_to_bitboard_transpose(move.to) & _pin_info.file_pin.small_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.file_pin.big_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_transpose(move.from) & _pin_info.file_pin.big_pin_pos) && !(_bitboard_impl->square_to_bitboard_transpose(move.to) & _pin_info.file_pin.big_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.diag_a1h8_pin.small_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_diag_a1h8(move.from) & _pin_info.diag_a1h8_pin.small_pin_pos) && !(_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _pin_info.diag_a1h8_pin.small_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.diag_a1h8_pin.big_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_diag_a1h8(move.from) & _pin_info.diag_a1h8_pin.big_pin_pos) && !(_bitboard_impl->square_to_bitboard_diag_a1h8(move.to) & _pin_info.diag_a1h8_pin.big_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.diag_a8h1_pin.small_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_diag_a8h1(move.from) & _pin_info.diag_a8h1_pin.small_pin_pos) && !(_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _pin_info.diag_a8h1_pin.small_pin_pos)) {
			return true;
		}
	}
	if (_pin_info.diag_a8h1_pin.big_square_slide != INVALID_SQUARE) {
		if ((_bitboard_impl->square_to_bitboard_diag_a8h1(move.from) & _pin_info.diag_a8h1_pin.big_pin_pos) && !(_bitboard_impl->square_to_bitboard_diag_a8h1(move.to) & _pin_info.diag_a8h1_pin.big_pin_pos)) {
			return true;
		}
	}

	return false;
}

bool PositionState::castling_checks_opponent_king(const move_info& move) const
{
	if (_white_to_play) {
		if (_black_king_position / 8 != 0) {
			if (move.to == C1) {
				if (_bitboard_impl->square_to_bitboard(D1) & _direct_check[ROOK_WHITE]) {
					return true;
				}
			}
			else {
				assert (move.to == G1);
				if (_bitboard_impl->square_to_bitboard(F1) & _direct_check[ROOK_WHITE]) {
					return true;
				}
			}
		}
		else {
			if (_bitboard_impl->square_to_bitboard(E1) & _direct_check[ROOK_WHITE]) {
				return true;
			}
		}
	}
	else {
		if (_white_king_position / 8 != 7) {
			if (move.to == C8) {
				if (_bitboard_impl->square_to_bitboard(D8) & _direct_check[ROOK_BLACK]) {
					return true;
				}
			}
			else {
				assert (move.to == G8);
				if (_bitboard_impl->square_to_bitboard(F8) & _direct_check[ROOK_BLACK]) {
					return true;
				}
			}
		}
		else {
			if (_bitboard_impl->square_to_bitboard(E8) & _direct_check[ROOK_WHITE]) {
				return true;
			}
		}
	}
	return false;
}
			

void PositionState::make_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	undo_move_info* undo_move = _move_stack.get_next_item();
	undo_move->from = move.from;
	undo_move->to = move.to;
	undo_move->moved_piece = pfrom;
	undo_move->captured_piece = pto;
	undo_move->en_passant_file = _en_passant_file;
	undo_move->white_left_castling = _white_left_castling;
	undo_move->white_right_castling = _white_right_castling;
	undo_move->black_left_castling = _black_left_castling;
	undo_move->black_right_castling = _black_right_castling;
	if (pfrom == PAWN_WHITE || pfrom == PAWN_BLACK) {
		if(move.to >= A8 || move.to <= H1) {
			make_promotion_move(move);
			undo_move->move_type = PROMOTION_MOVE;
		}
		else if (std::abs(move.from - move.to) == 16) {
			make_en_passant_move(move);
			undo_move->move_type = EN_PASSANT_MOVE;
		}
		else if (((move.to - move.from) % 8) && pto == ETY_SQUARE) {
			make_en_passant_capture(move);
			undo_move->move_type = EN_PASSANT_CAPTURE;
		}
		else {
			make_normal_move(move);
			undo_move->move_type = NORMAL_MOVE;
		}
	}
	else if ((pfrom == KING_WHITE || pfrom == KING_BLACK) && std::abs(move.from % 8 - move.to % 8) > 1) {
		make_castling_move(move);
		undo_move->move_type = CASTLING_MOVE;
	}		
	else {
		make_normal_move(move);
		undo_move->move_type = NORMAL_MOVE;
	}

	update_castling_rights(move);
	update_game_status();
	// TODO : check if opponents king is under attack and update the variable accordingly
	
	_white_to_play = !_white_to_play;
	_zob_key ^= _zob_key_impl->get_if_black_to_play_key();
}

void PositionState::make_normal_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (_white_to_play) {			
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		if (pto != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, BLACK);
			_pst_value -= calculate_pst_value(pto, move.to);
			--_piece_count[pto];
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(pto, move.to);
			_material_zob_key ^= _zob_key_impl->get_material_key(pto, _piece_count[pto] + 1);
		}
		if (pfrom == KING_WHITE) {
			_white_king_position = move.to;
		}
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		if (pto != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, WHITE);
			_pst_value -= calculate_pst_value(pto, move.to);
			--_piece_count[pto];
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(pto, move.to);
			_material_zob_key ^= _zob_key_impl->get_material_key(pto, _piece_count[pto] + 1);
		}
		if (pfrom == KING_BLACK) {
			_black_king_position = move.to;
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pst_value -= calculate_pst_value(pfrom, move.from);
	_pst_value += calculate_pst_value(pfrom, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.to);
	if (_en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
		_en_passant_file = -1;				
	}	
}

//Castling is assumed to be King's move
void PositionState::make_castling_move(const move_info& move)
{
	if (_white_to_play) {
		assert(move.from == E1);
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_WHITE;
		_pst_value -= calculate_pst_value(KING_WHITE, move.from);
		_pst_value += calculate_pst_value(KING_WHITE, move.to);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_WHITE, move.from);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_WHITE, move.to);
		_white_king_position = move.to;
		if (move.to == C1) {
			remove_piece_from_bitboards(A1, WHITE);
			add_piece_to_bitboards(D1, WHITE);	
			_board[A1 / 8][A1 % 8] = ETY_SQUARE;
			_board[D1 / 8][D1 % 8] = ROOK_WHITE;
			_pst_value -= calculate_pst_value(ROOK_WHITE, A1);
			_pst_value += calculate_pst_value(ROOK_WHITE, D1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, A1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, D1);
		}	
		else {
			assert(move.to == G1);
			remove_piece_from_bitboards(H1, WHITE);
			add_piece_to_bitboards(F1, WHITE);	
			_board[H1 / 8][H1 % 8] = ETY_SQUARE;
			_board[F1 / 8][F1 % 8] = ROOK_WHITE;
			_pst_value -= calculate_pst_value(ROOK_WHITE, H1);
			_pst_value += calculate_pst_value(ROOK_WHITE, F1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, H1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, F1);

		}
	}
	else {
		assert(move.from == E8);
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		_board[move.from / 8][move.from % 8] = ETY_SQUARE;
		_board[move.to / 8][move.to % 8] = KING_BLACK;
		_pst_value -= calculate_pst_value(KING_BLACK, move.from);
		_pst_value += calculate_pst_value(KING_BLACK, move.to);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_BLACK, move.from);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_BLACK, move.to);
		_black_king_position = move.to;
		if (move.to == C8) {
			remove_piece_from_bitboards(A8, BLACK);
			add_piece_to_bitboards(D8, BLACK);	
			_board[A8 / 8][A8 % 8] = ETY_SQUARE;
			_board[D8 / 8][D8 % 8] = ROOK_BLACK;
			_pst_value -= calculate_pst_value(ROOK_BLACK, A8);
			_pst_value += calculate_pst_value(ROOK_BLACK, D8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, A8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, D8);
		}	
		else {
			assert(move.to == G8);
			remove_piece_from_bitboards(H8, BLACK);
			add_piece_to_bitboards(F8, BLACK);	
			_board[H8 / 8][H8 % 8] = ETY_SQUARE;
			_board[F8 / 8][F8 % 8] = ROOK_BLACK;
			_pst_value -= calculate_pst_value(ROOK_BLACK, H8);
			_pst_value += calculate_pst_value(ROOK_BLACK, F8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, H8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, F8);
		}
	}
	if (_en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
		_en_passant_file = -1;	
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
	_pst_value -= calculate_pst_value(pfrom, move.from);
	_pst_value += calculate_pst_value(pfrom, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.to);
	if (_en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
	}
	
	_en_passant_file = move.from % 8;
	_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
}

void PositionState::make_en_passant_capture(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	if (_white_to_play) {
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		remove_piece_from_bitboards((Square) (move.to - 8), BLACK);
		--_piece_count[PAWN_BLACK];
		_board[move.to / 8 - 1][move.to % 8] = ETY_SQUARE;
		_pst_value -= calculate_pst_value(PAWN_BLACK, (Square) (move.to - 8));
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(PAWN_BLACK, (Square) (move.to - 8));
		_material_zob_key ^= _zob_key_impl->get_material_key(PAWN_BLACK, _piece_count[PAWN_BLACK] + 1);
	}
	else {
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		remove_piece_from_bitboards((Square) (move.to + 8), WHITE);
		--_piece_count[PAWN_WHITE];
		_board[move.to / 8 + 1][move.to % 8] = ETY_SQUARE;
		_pst_value -= calculate_pst_value(PAWN_WHITE, (Square) (move.to + 8));
		_material_zob_key ^= _zob_key_impl->get_material_key(PAWN_WHITE, _piece_count[PAWN_WHITE] + 1);
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = pfrom;
	_pst_value -= calculate_pst_value(pfrom, move.from);
	_pst_value += calculate_pst_value(pfrom, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.to);
	if (_en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
		_en_passant_file = -1;				
	}
}

void PositionState::make_promotion_move(const move_info& move)
{
	Piece pfrom = _board[move.from / 8][move.from % 8];
	Piece pto = _board[move.to / 8][move.to % 8];
	if (_white_to_play) {
		assert(move.from >= A7 && move.from <= H7);
		remove_piece_from_bitboards(move.from, WHITE);
		add_piece_to_bitboards(move.to, WHITE);
		if (pto != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, BLACK);
			_pst_value -= calculate_pst_value(pto, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(pto, move.to);
			--_piece_count[pto];
			_material_zob_key ^= _zob_key_impl->get_material_key(pto, _piece_count[pto] + 1);
		}
	}
	else {
		assert(move.from >= A2 && move.from <= H2);
		remove_piece_from_bitboards(move.from, BLACK);
		add_piece_to_bitboards(move.to, BLACK);
		if (pto != ETY_SQUARE) {
			remove_piece_from_bitboards(move.to, WHITE);
			_pst_value -= calculate_pst_value(pto, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(pto, move.to);
			--_piece_count[pto];
			_material_zob_key ^= _zob_key_impl->get_material_key(pto, _piece_count[pto] + 1);
		}
	}
	_board[move.from / 8][move.from % 8] = ETY_SQUARE;
	_board[move.to / 8][move.to % 8] = move.promoted;
	--_piece_count[pfrom];
	++_piece_count[move.promoted];
	_pst_value -= calculate_pst_value(pfrom, move.from);
	_pst_value += calculate_pst_value(move.promoted, move.to);

	_zob_key ^= _zob_key_impl->get_piece_at_square_key(pfrom, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.promoted, move.to);
	_material_zob_key ^= _zob_key_impl->get_material_key(pfrom, _piece_count[pfrom] + 1);
	_material_zob_key ^= _zob_key_impl->get_material_key(move.promoted, _piece_count[move.promoted]);

	if (_en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
		_en_passant_file = -1;
	}
}

// Updates castling variables by checking whether king and rooks 
// are their designated positions after the move
void PositionState::update_castling_rights(const move_info& move)
{
	if (_white_to_play) {
		if (move.from == E1) {
			if (_white_left_castling) {
				_white_left_castling = false;
				_zob_key ^= _zob_key_impl->get_white_left_castling_key();
			}
			if (_white_right_castling) {
				_white_right_castling = false;
				_zob_key ^= _zob_key_impl->get_white_right_castling_key();
			}
		}
		else {
			if (_white_left_castling && move.from == A1) {
				_white_left_castling = false;
				_zob_key ^= _zob_key_impl->get_white_left_castling_key();
			}
			else if (_white_right_castling && move.from == H1) {
				_white_right_castling = false;
				_zob_key ^= _zob_key_impl->get_white_right_castling_key();
			}
			
			if (_black_left_castling && move.to == A8) {
				_black_left_castling = false;
				_zob_key ^= _zob_key_impl->get_black_left_castling_key();
			}
			else if (_black_right_castling && move.to == H8) {
				_black_right_castling = false;
				_zob_key ^= _zob_key_impl->get_black_right_castling_key();
			}
		}
	}
	else {
		if (move.from == E8) {
			if (_black_left_castling) {
				_black_left_castling = false;
				_zob_key ^= _zob_key_impl->get_black_left_castling_key();
			}
			if (_black_right_castling) {
				_black_right_castling = false;
				_zob_key ^= _zob_key_impl->get_black_right_castling_key();
			}
		}
		else {
			if (_black_left_castling && move.from == A8) {
				_black_left_castling = false;
				_zob_key ^= _zob_key_impl->get_black_left_castling_key();
			}
			else if (_black_right_castling && move.from == H8) {
				_black_right_castling = false;
				_zob_key ^= _zob_key_impl->get_black_right_castling_key();
			}
			
			if (_white_left_castling && move.to == A1) {
				_white_left_castling = false;
				_zob_key ^= _zob_key_impl->get_white_left_castling_key();
			}
			else if (_white_right_castling && move.to == H1) {
				_white_right_castling = false;
				_zob_key ^= _zob_key_impl->get_white_right_castling_key();
			}
		}
	}
}



// Adds a piece into all 4 occupation bitboards in the appropriate position
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

// Removes a piece from all 4 occupation bitboards in the appropriate position
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

int PositionState::calculate_pst_value(Piece p, Square s) const
{
	if (_is_middle_game) {
		return PST_MIDDLE_VALUE[p][s];
	}
	else {
		return PST_END_VALUE[p][s];
	}
}

// Updates the status of the game by setting 
// is_middle_game variable true or false
// TODO: Needs improvement on classifing middle and end game
void PositionState::update_game_status()
{
	if (_piece_count[QUEEN_WHITE] == 0 && _piece_count[QUEEN_BLACK] == 0) {
		_is_middle_game = false;
	}
}


void PositionState::undo_move()
{
	const undo_move_info* move = _move_stack.pop();
	switch(move->move_type) {
		case NORMAL_MOVE: 
			undo_normal_move(*move);
			break;
		case PROMOTION_MOVE:
			undo_promotion_move(*move);
			break;
		case CASTLING_MOVE:
			undo_castling_move(*move);
			break;
		case EN_PASSANT_MOVE:
			undo_en_passant_move(*move);
			break;
		case EN_PASSANT_CAPTURE:
			undo_en_passant_capture(*move);
			break;
		default:
			assert(move->move_type >= NORMAL_MOVE && move->move_type <= EN_PASSANT_CAPTURE);
			break;
	}

	revert_castling_rights(*move);
	update_game_status();
	
	_white_to_play = !_white_to_play;
	_zob_key ^= _zob_key_impl->get_if_black_to_play_key();
}

void PositionState::undo_normal_move(const undo_move_info& move)
{
	if (_white_to_play) {			
		remove_piece_from_bitboards(move.to, BLACK);
		add_piece_to_bitboards(move.from, BLACK);
		if (move.captured_piece != ETY_SQUARE) {
			add_piece_to_bitboards(move.to, WHITE);
			++_piece_count[move.captured_piece];
			_pst_value += calculate_pst_value(move.captured_piece, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.captured_piece, move.to);
			_material_zob_key ^= _zob_key_impl->get_material_key(move.captured_piece, _piece_count[move.captured_piece]);
		}
		if (move.moved_piece == KING_BLACK) {
			_black_king_position = move.from;
		}
	}
	else {
		remove_piece_from_bitboards(move.to, WHITE);
		add_piece_to_bitboards(move.from, WHITE);
		if (move.captured_piece != ETY_SQUARE) {
			add_piece_to_bitboards(move.to, BLACK);
			++_piece_count[move.captured_piece];
			_pst_value += calculate_pst_value(move.captured_piece, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.captured_piece, move.to);
			_material_zob_key ^= _zob_key_impl->get_material_key(move.captured_piece, _piece_count[move.captured_piece]);
		}
		if (move.moved_piece == KING_WHITE) {
			_white_king_position = move.from;
		}
	}
	_board[move.from / 8][move.from % 8] = move.moved_piece;
	_board[move.to / 8][move.to % 8] = move.captured_piece;
	_pst_value -= calculate_pst_value(move.moved_piece, move.to);
	_pst_value += calculate_pst_value(move.moved_piece, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.from);
	if (move.en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(move.en_passant_file);
		_en_passant_file = move.en_passant_file;
	}
}

void PositionState::undo_castling_move(const undo_move_info& move)
{
	if (_white_to_play) {
		assert(move.from == E8);
		assert(move.moved_piece == KING_BLACK);
		remove_piece_from_bitboards(move.to, BLACK);
		add_piece_to_bitboards(move.from, BLACK);
		_board[move.from / 8][move.from % 8] = KING_BLACK;
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_pst_value -= calculate_pst_value(KING_BLACK, move.to);
		_pst_value += calculate_pst_value(KING_BLACK, move.from);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_BLACK, move.to);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_BLACK, move.from);
		_black_king_position = move.from;
		if (move.to == C8) {
			remove_piece_from_bitboards(D8, BLACK);
			add_piece_to_bitboards(A8, BLACK);	
			_board[D8 / 8][D8 % 8] = ETY_SQUARE;
			_board[A8 / 8][A8 % 8] = ROOK_BLACK;
			_pst_value -= calculate_pst_value(ROOK_BLACK, D8);
			_pst_value += calculate_pst_value(ROOK_BLACK, A8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, D8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, A8);
		}	
		else {
			assert(move.to == G8);
			remove_piece_from_bitboards(F8, BLACK);
			add_piece_to_bitboards(H8, BLACK);	
			_board[F8 / 8][F8 % 8] = ETY_SQUARE;
			_board[H8 / 8][H8 % 8] = ROOK_BLACK;
			_pst_value -= calculate_pst_value(ROOK_BLACK, F8);
			_pst_value += calculate_pst_value(ROOK_BLACK, H8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, F8);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_BLACK, H8);
		}
	}
	else {
		assert(move.from == E1);
		assert(move.moved_piece == KING_WHITE); 
		remove_piece_from_bitboards(move.to, WHITE);
		add_piece_to_bitboards(move.from, WHITE);
		_board[move.to / 8][move.to % 8] = ETY_SQUARE;
		_board[move.from / 8][move.from % 8] = KING_WHITE;
		_pst_value -= calculate_pst_value(KING_WHITE, move.to);
		_pst_value += calculate_pst_value(KING_WHITE, move.from);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_WHITE, move.to);
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(KING_WHITE, move.from);
		_white_king_position = move.from;
		if (move.to == C1) {
			remove_piece_from_bitboards(D1, WHITE);
			add_piece_to_bitboards(A1, WHITE);	
			_board[D1 / 8][D1 % 8] = ETY_SQUARE;
			_board[A1 / 8][A1 % 8] = ROOK_WHITE;
			_pst_value -= calculate_pst_value(ROOK_WHITE, D1);
			_pst_value += calculate_pst_value(ROOK_WHITE, A1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, D1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, A1);
		}	
		else {
			assert(move.to == G1);
			remove_piece_from_bitboards(F1, WHITE);
			add_piece_to_bitboards(H1, WHITE);	
			_board[F1 / 8][F1 % 8] = ETY_SQUARE;
			_board[H1 / 8][H1 % 8] = ROOK_WHITE;
			_pst_value -= calculate_pst_value(ROOK_WHITE, F1);
			_pst_value += calculate_pst_value(ROOK_WHITE, H1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, F1);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(ROOK_WHITE, H1);

		}
	}
	if (move.en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(move.en_passant_file);
		_en_passant_file = move.en_passant_file;	
	}	
}

void PositionState::undo_en_passant_move(const undo_move_info& move)
{
	if (_white_to_play) {
		remove_piece_from_bitboards(move.to, BLACK);
		add_piece_to_bitboards(move.from, BLACK);
	}
	else {
		remove_piece_from_bitboards(move.to, WHITE);
		add_piece_to_bitboards(move.from, WHITE);
	}
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = ETY_SQUARE;
	_board[move.from / 8][move.from % 8] = move.moved_piece;
	_pst_value -= calculate_pst_value(move.moved_piece, move.to);
	_pst_value += calculate_pst_value(move.moved_piece, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.from);
	_zob_key ^= _zob_key_impl->get_en_passant_key(_en_passant_file);
	if (move.en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(move.en_passant_file);
	}
	_en_passant_file = move.en_passant_file;
}

void PositionState::undo_en_passant_capture(const undo_move_info& move)
{
	if (_white_to_play) {
		remove_piece_from_bitboards(move.to, BLACK);
		add_piece_to_bitboards(move.from, BLACK);
		add_piece_to_bitboards((Square) (move.to + 8), WHITE);
		++_piece_count[PAWN_WHITE];
		_board[move.to / 8 + 1][move.to % 8] = PAWN_WHITE;
		_pst_value += calculate_pst_value(PAWN_WHITE, (Square) (move.to + 8));
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(PAWN_WHITE, (Square) (move.to + 8));
		_material_zob_key ^= _zob_key_impl->get_material_key(PAWN_WHITE, _piece_count[PAWN_WHITE]);
	}
	else {
		remove_piece_from_bitboards(move.to, WHITE);
		add_piece_to_bitboards(move.from, WHITE);
		add_piece_to_bitboards((Square) (move.to - 8), BLACK);
		++_piece_count[PAWN_BLACK];
		_board[move.to / 8 - 1][move.to % 8] = PAWN_BLACK;
		_pst_value += calculate_pst_value(PAWN_BLACK, (Square) (move.to - 8));
		_zob_key ^= _zob_key_impl->get_piece_at_square_key(PAWN_BLACK, (Square) (move.to - 8));
		_material_zob_key ^= _zob_key_impl->get_material_key(PAWN_BLACK, _piece_count[PAWN_BLACK]);
	}
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = ETY_SQUARE;
	_board[move.from / 8][move.from % 8] = move.moved_piece;
	_pst_value -= calculate_pst_value(move.moved_piece, move.to);
	_pst_value += calculate_pst_value(move.moved_piece, move.from);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.from);
	if (move.en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(move.en_passant_file);
		_en_passant_file = move.en_passant_file;				
	}
}

void PositionState::undo_promotion_move(const undo_move_info& move)
{
	if (_white_to_play) {
		assert(move.from >= A2 && move.from <= H2);
		remove_piece_from_bitboards(move.to, BLACK);
		add_piece_to_bitboards(move.from, BLACK);
		if (move.captured_piece != ETY_SQUARE) {
			add_piece_to_bitboards(move.to, WHITE);
			_pst_value += calculate_pst_value(move.captured_piece, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.captured_piece, move.to);
			++_piece_count[move.captured_piece];
			_material_zob_key ^= _zob_key_impl->get_material_key(move.captured_piece, _piece_count[move.captured_piece]);
		}
	}
	else {
		assert(move.from >= A7 && move.from <= H7);
		remove_piece_from_bitboards(move.to, WHITE);
		add_piece_to_bitboards(move.from, WHITE);
		if (move.captured_piece != ETY_SQUARE) {
			add_piece_to_bitboards(move.to, BLACK);
			_pst_value += calculate_pst_value(move.captured_piece, move.to);
			_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.captured_piece, move.to);
			++_piece_count[move.captured_piece];
			_material_zob_key ^= _zob_key_impl->get_material_key(move.captured_piece, _piece_count[move.captured_piece]);
		}
	}
	Piece promoted = _board[move.to / 8][move.to % 8];
	assert(_board[move.to / 8][move.to % 8] != ETY_SQUARE);	
	_board[move.to / 8][move.to % 8] = move.captured_piece;
	_board[move.from / 8][move.from % 8] = move.moved_piece;
	--_piece_count[promoted];
	++_piece_count[move.moved_piece];
	_pst_value -= calculate_pst_value(promoted, move.to);
	_pst_value += calculate_pst_value(move.moved_piece, move.from);

	_zob_key ^= _zob_key_impl->get_piece_at_square_key(promoted, move.to);
	_zob_key ^= _zob_key_impl->get_piece_at_square_key(move.moved_piece, move.from);
	_material_zob_key ^= _zob_key_impl->get_material_key(promoted, _piece_count[promoted] + 1);
	_material_zob_key ^= _zob_key_impl->get_material_key(move.moved_piece, _piece_count[move.moved_piece]);

	if (move.en_passant_file != -1) {
		_zob_key ^= _zob_key_impl->get_en_passant_key(move.en_passant_file);
		_en_passant_file = move.en_passant_file;
	}
}

void PositionState::revert_castling_rights(const undo_move_info& move)
{

	if (!_white_left_castling && move.white_left_castling) {
		_zob_key ^= _zob_key_impl->get_white_left_castling_key();
		_white_left_castling = true;
	}
	if (!_white_right_castling && move.white_right_castling) {
		_zob_key ^= _zob_key_impl->get_white_right_castling_key();
		_white_right_castling = true;
	}
	if (!_black_left_castling && move.black_left_castling) {
		_zob_key ^= _zob_key_impl->get_black_left_castling_key();
		_black_left_castling = true;
	}
	if (!_black_right_castling && move.black_right_castling) {
		_zob_key ^= _zob_key_impl->get_black_right_castling_key();
		_black_right_castling = true;
	}
}
	 
PositionState::MoveStack::MoveStack() :
_stack_size(0)
{
}

bool PositionState::MoveStack::isEmpty() const
{
	return _stack_size == 0;
}

uint32_t PositionState::MoveStack::get_size() const
{
	return _stack_size;
}

// Returns the pointer to the next item which should be updated
// and then pushed into stack
// The value held in the variable to which the pointer points to is undefined  
PositionState::undo_move_info* PositionState::MoveStack::get_next_item()
{
	assert(_stack_size != MOVE_STACK_CAPACITY);
	return _move_stack + (_stack_size++);
}

const PositionState::undo_move_info* PositionState::MoveStack::pop()
{
	assert(!isEmpty());
	return _move_stack + (--_stack_size);
}

const std::string PositionState::get_state_FEN() const
{
	std::string fen;
	construct_material_FEN(fen);
	fen.push_back(' ');
	construct_right_to_play_FEN(fen);
	fen.push_back(' ');
	construct_castling_rights_FEN(fen);
	fen.push_back(' ');
	construct_en_passant_file_FEN(fen);
	fen.push_back(' ');
	construct_move_count_FEN(fen);

	return fen;
}

void PositionState::construct_material_FEN(std::string& fen) const
{
	for (int rank = 7; rank >= 0; --rank) {
		unsigned int ety_square_count  = 0;
		for (unsigned int file = 0; file < 8; ++file) {
			Piece p = _board[rank][file];
			if(p != ETY_SQUARE) {
				if (ety_square_count != 0) {
					fen.push_back('0' + ety_square_count);
					ety_square_count = 0;
				}
				switch(p) {
					case PAWN_WHITE:
						fen.push_back('P');
						break;
					case KNIGHT_WHITE:
						fen.push_back('N');
						break;
					case BISHOP_WHITE:
						fen.push_back('B');
						break;
					case ROOK_WHITE:
						fen.push_back('R');
						break;
					case QUEEN_WHITE:
						fen.push_back('Q');
						break;
					case KING_WHITE:
						fen.push_back('K');
						break;
					case PAWN_BLACK:
						fen.push_back('p');
						break;
					case KNIGHT_BLACK:
						fen.push_back('n');
						break;
					case BISHOP_BLACK:
						fen.push_back('b');
						break;
					case ROOK_BLACK:
						fen.push_back('r');
						break;
					case QUEEN_BLACK:
						fen.push_back('q');
						break;
					case KING_BLACK:
						fen.push_back('k');
						break;
					default:
						assert(false);
				}
			}
			else {
				++ety_square_count;
			}
		}
		
		if (ety_square_count != 0) {
			fen.push_back('0' + ety_square_count);
			ety_square_count = 0;
		}
		if (rank != 0) {
			fen.push_back('/');
		}
	}
}

void PositionState::construct_right_to_play_FEN(std::string& fen) const
{
	if (_white_to_play) {
		fen.push_back('w');
	}
	else {
		fen.push_back('b');
	}
}

void PositionState::construct_castling_rights_FEN(std::string& fen) const
{
	bool castling_allowed = false;
	if (_white_right_castling) {
		fen.push_back('K');
		castling_allowed = true;
	}
	if (_white_left_castling) {
		fen.push_back('Q');
		castling_allowed = true;
	}
	if (_black_right_castling) {
		fen.push_back('k');
		castling_allowed = true;
	}
	if (_black_left_castling) {
		fen.push_back('q');
		castling_allowed = true;
	}
	if (!castling_allowed) {
		fen.push_back('-');
	}
}

void PositionState::construct_en_passant_file_FEN(std::string& fen) const
{
	if(_en_passant_file == -1) {
		fen.push_back('-');
	}
	else {
		fen.push_back('a' + _en_passant_file);
		if (_white_to_play) {
			fen.push_back('6');
		}
		else {
			fen.push_back('3');
		}
	}
}

void PositionState::construct_move_count_FEN(std::string& fen) const
{
	unsigned int halfmove_count = _halfmove_clock;
	std::string hc_rev;
	do {
		hc_rev.push_back('0' + halfmove_count % 10);
		halfmove_count /= 10;
	} while (halfmove_count != 0);
	for (int i = hc_rev.size() - 1; i >= 0; --i) {
		fen.push_back(hc_rev[i]);
	}
	fen.push_back(' ');
	unsigned int fullmove_count = _fullmove_count;
	std::string fc_rev;
	do {
		fc_rev.push_back('0' + fullmove_count % 10);
		fullmove_count /= 10;
	} while (fullmove_count != 0);
	for (int i = fc_rev.size() - 1; i >=0; --i) {
		fen.push_back(fc_rev[i]);
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
