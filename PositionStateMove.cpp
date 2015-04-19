#include "PositionState.h"

namespace pismo
{

bool PositionState::move_is_legal_full(Square from, Square to)
{
	if(whiteToPlay)
		switch(board[from / 8][from % 8])
		{
			case PAWN_WHITE: 
				return move_legal_pawn(from, to , WHITE);
			case KNIGHT_WHITE: 
				return move_legal_knight(from, to, WHITE);
			case BISHOP_WHITE:
				return move_legal_bishop(from, to, WHITE);
			case ROOK_WHITE:
				return move_legal_rook(from, to, WHITE);
			case QUEEN_WHITE:
				return move_legal_queen(from, to, WHITE);
			case KING_WHITE:
				return move_legal_king(from, to, WHITE);
		}
	else
		switch(board[from / 8][from % 8])
		{
			case PAWN_BLACK:
				return move_legal_pawn(from, to, BLACK);
			case KNIGHT_BLACK:
				return move_legal_knight(from, to, BLACK);
			case BISHOP_BLACK:
				return move_legal_bishop(from, to, BLACK);
			case ROOK_BLACK:
				return move_legal_rook(from, to, BLACK);
			case QUEEN_BLACK:
				return move_legal_queen(from, to, BLACK);
			case KING_WHITE:
				return move_legal_king(from, to, BLACK);
		}

	return false;
}
}
