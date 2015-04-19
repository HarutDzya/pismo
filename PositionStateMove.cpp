#include "PositionState.h"

namespace pismo
{

bool PositionState::move_is_legal_full(Square from, Square to) const
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

bool PositionState::move_legal_pawn(Square from, Square to, Color clr) const
{
	Bitboard tmp = 1;
	Bitboard init;
	if (clr == WHITE)
	{
		if ((tmp << (from + 8)) & (tmp << to))
			return true;
		init = 0x000000000000FF00;
		if (((tmp << (from + 16)) & (tmp << to)) && ((tmp << from) & init) &&
		!((tmp << (from + 8)) & whitePieces) && !((tmp << (from + 8)) & blackPieces))
			return true;
		if ((((tmp << (from + 7)) & (tmp << to)) && ((tmp << (from + 7)) & blackPieces)) ||
		(((tmp << (from + 9)) & (tmp << to)) && ((tmp << (from + 9)) & blackPieces)))
			return true;
		if (en_passant(from, to, WHITE))
			return true;
		return false;
	}
	else
	{
	
		if ((tmp << (from - 8)) & (tmp << to))
			return true;
		init = 0x00FF000000000000;
		if (((tmp << (from - 16)) & (tmp << to)) && ((tmp << from) & init) &&
		!((tmp << (from - 8)) & whitePieces) && !((tmp << (from - 8)) & blackPieces))
			return true;
		if ((((tmp << (from - 7)) & (tmp << to)) && ((tmp << (from - 7)) & whitePieces)) ||
		(((tmp << (from - 9)) & (tmp << to)) && ((tmp << (from - 9)) & whitePieces)))
			return true;
		if (en_passant(from, to, BLACK))
			return true;
		return false;
	}
}

bool PositionState::en_passant(Square from, Square to, Color clr) const
{
	if(enPassantFile != -1)
	{
		Bitboard tmp = 1;
		if (clr == WHITE)
		{
			if ((tmp << from) & ((tmp << (4 * 8 + enPassantFile + 1)) ||
			(tmp << (4 * 8 + enPassantFile - 1))))
				if ((tmp << from) & (tmp << (5 * 8 + enPassantFile)))
					return true;
		}
		else
		{
			if ((tmp << from) & ((tmp << (3 * 8 + enPassantFile + 1)) ||
			(tmp << (3 * 8 + enPassantFile - 1))))
				if ((tmp << from) & (tmp << (2 * 8 + enPassantFile)))
					return true;
		}
 	}
}

bool PositionState::move_legal_knight(Square from, Square to, Color clr) const
{
	return true;
}

bool PositionState::move_legal_bishop(Square from, Square to, Color clr) const
{
	return true;
}

bool PositionState::move_legal_rook(Square from, Square to, Color clr) const
{
	return true;
}

bool PositionState::move_legal_queen(Square from, Square to, Color clr) const
{
	return true;
}

bool PositionState::move_legal_king(Square from, Square to, Color clr) const
{
	return true;
}
}
