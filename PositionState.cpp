#include "PositionState.h"
#include <iostream>

namespace pismo
{

PositionState::PositionState() : whiteToPlay(true), kingUnderAttack(false), whitePieces(0), blackPieces(0)
{
	for(int i = 0; i < 8; ++i)
		for(int j = 0; j < 8; ++j)
			board[i][j] = ETY_SQUARE;
}

void PositionState::set_piece(Square s, Piece p)
{
	board[s / 8][s % 8] = p;
	Bitboard tmp = 1;
	if(p <= KING_WHITE)
		whitePieces |= (tmp << s);
	else
		blackPieces |= (tmp << s);
}

void PositionState::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	for(std::size_t i = 0; i < pieces.size(); ++i)
		set_piece(pieces[i].first, pieces[i].second);
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

void PositionState::move(Square from, Square to)
{
	if (move_is_legal(from, to))
	{
		Bitboard tmp = 1;
		Piece pfrom = board[from / 8][from % 8];
		if (pfrom <= KING_WHITE)
		{
			whitePieces ^= (tmp << from);
			whitePieces |= (tmp << to);
		}
		else
		{
			blackPieces ^= (tmp << from);
			blackPieces |= (tmp << to);
		}
		board[from / 8][from % 8] = ETY_SQUARE;
		board[to / 8][to % 8] = pfrom;
		whiteToPlay = !whiteToPlay;
	}
}

void PositionState::print_double() const
{
	std::cout << "White pieces:" << std::endl;
	for(int i = 7; i >= 0; --i)
	{
		for(int j = 0; j < 8; ++j)
		{
			if ((whitePieces >> (i * 8 + j)) & 1)	
				switch(board[i][j])
				{
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
				}
			else
				std::cout << "E ";
		if (j == 7)
			std::cout << std::endl;
		}
	}

	std::cout << "Black pieces:" << std::endl;
	for(int i = 7; i >= 0; --i)
	{
		for(int j = 0; j < 8; ++j)
		{
			if ((blackPieces >> (i * 8 + j)) & 1)	
				switch(board[i][j])
				{
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
				}
			else
				std::cout << "E ";
		if (j == 7)
			std::cout << std::endl;
		}
	}
}

void PositionState::print_single() const
{
	std::cout << "Complete board" << std::endl; 
	for(int i = 7; i >= 0; --i)
	{
		for(int j = 0; j < 8; ++j)
		{
			switch(board[i][j])
			{
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
			
			if (j == 7)
				std::cout << std::endl;
		}
	}
}
}
