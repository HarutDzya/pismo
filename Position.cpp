#include "Position.h"
#include <iostream>

namespace pismo
{

Position::Position() : whiteToPlay(true), whitePieces(0), blackPieces(0)
{
}

void Position::set_piece(Square s, Piece p)
{
	board[s / 8][s % 8] = p;
	Bitboard tmp = 1;
	if(p <= KING_WHITE)
		whitePieces = whitePieces | (tmp << s);
	else
		blackPieces = blackPieces | (tmp << s);
}

void Position::init_position(const std::vector<std::pair<Square, Piece> >& pieces)
{
	for(std::size_t i = 0; i < pieces.size(); ++i)
		set_piece(pieces[i].first, pieces[i].second);
}

bool Position::move_is_legal(Square from, Square to) const
{
	Bitboard tmp = 1;
	return ((whitePieces & (tmp << from)) || (blackPieces & (tmp << from))
	&& (!(whitePieces & (tmp << to)) && !(blackPieces & (tmp << to)))); 
}

void Position::move(Square from, Square to)
{
	if (move_is_legal(from, to))
	{
		Bitboard tmp = 1;
		Piece pfrom = board[from / 8][from % 8];
		if (pfrom <= KING_WHITE)
		{
			whitePieces = whitePieces ^ (tmp << from);
			whitePieces = whitePieces | (tmp << to);
		}
		else
		{
			blackPieces = blackPieces ^ (tmp << from);
			blackPieces = blackPieces | (tmp << to);
		}
		board[to / 8][to % 8] = pfrom;
	}
}

void Position::print() const
{
	std::cout << "White pieces:" << std::endl;
	for(int i = 63; i >= 0; --i)
	{
		if ((whitePieces >> i) & 1)	
			switch(board[i / 8][i % 8])
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
		if (i % 8 == 0)
			std::cout << std::endl;
	}

	std::cout << "Black pieces:" << std::endl;
	for(int i = 63; i >= 0; --i)
	{
		if ((blackPieces >> i) & 1)	
			switch(board[i / 8][i % 8])
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
		if (i % 8 == 0)
			std::cout << std::endl;
	}
}
}
