#ifndef POSITIONSTATE_H_
#define POSITIONSTATE_H_

#include "utils.h"
#include <vector>
#include <string>

namespace pismo
{

class BitboardImpl;
class ZobKeyImpl;

const unsigned int MOVE_STACK_CAPACITY = 20;

class PositionState
{
public:
	PositionState();
	~PositionState();

	void initPosition(const std::vector<std::pair<Square, Piece> >& pieces); 

	// Initializes the state using Forsyth-Edwards notation string as an input
	// the FEN should have 6 fields separated by whitespaces
	void initPositionFEN(const std::string& fen);

	/*
	Checks all the rules of the game for legality of the move
	even if it involves capture of the opponent piece
	*/
	bool moveIsLegal(const moveInfo& move) const;
	
	/*
	Makes a move if the move if legal according to the moveIsLegal
	method
	*/
	void makeMove(const moveInfo& move);

	/*
	Makes an undo move of the last made move, by reverting all
	state variables to the previous state
	*/
	void undoMove();

	/* Checks to see whether the move checks opponent king
	   by either direct check or discovered check.
	   In order for this function to work the updateDirectCheckArray()
	   and updateDiscoveredChecks() should be invoked first.
	*/
	bool moveChecksOpponentKing(const moveInfo& move) const;
	
	void updateDirectCheckArray();
	void updateDiscoveredChecks();

	
	/*
	Prints board for white pieces using information from 
	_whitePieces Bitboard
	*/
	void printWhitePieces() const;
	
	/*
	Prints board for black pieces using information from 
	_blackPieses Bitboard
	*/
	void printBlackPieces() const;

	/*
	Prints single board for both pieces using information
	from _board array
	*/
	void printBoard() const;

	/* Constructs state Forsyth-Edwards notation
	 * and returns as a string
	 */
	const std::string getStateFEN() const;

	/*
	Prints possible moves from Square from on the board
	*/
	void printPossibleMoves(Square from) const;

	ZobKey getZobKey() const {return _zobKey;}

	Piece const (&getBoard()const)[8][8] {return _board;}

	unsigned int getPieceCount(Piece p) const {return _pieceCount[p];}

	ZobKey getMaterialZobKey() const {return _materialZobKey;}

	int getPstValue() const {return _pstValue;}

	bool whiteToPlay() const {return _whiteToPlay;}

//private member functions
private:
	void setPiece(Square s, Piece p);
	bool initPositionIsValid(const std::vector<std::pair<Square, Piece> >& pieces) const;
	void initMaterialFEN(const std::string& fen, unsigned int& charCount);
	void initRightToPlayFEN(const std::string& fen, unsigned int& charCount);
	void initCastlingRightsFEN(const std::string& fen, unsigned int& charCount);
	void initEnPassantFileFEN(const std::string& fen, unsigned int& charCount);
	void initMoveCountFEN(const std::string& fen, unsigned int& charCount);

	bool pawnMoveIsLegal(const moveInfo& move, bool& isEnPassantCapture) const;
	bool knightMoveIsLegal(const moveInfo& move) const;
	bool bishopMoveIsLegal(const moveInfo& move) const;
	bool rookMoveIsLegal(const moveInfo& move) const;
	bool queenMoveIsLegal(const moveInfo& move) const;
	bool kingMoveIsLegal(const moveInfo& move) const;
	bool enPassantCaptureIsLegal(const moveInfo& move) const;
	bool castlingIsLegal(const moveInfo& move) const;
	
	Bitboard squaresUnderAttack(Color attackedColor) const;

	void makeLazyMove(const moveInfo& move, bool isEnPassantCapture, Piece& capturedPiece) const;
	void undoLazyMove(const moveInfo& move, bool isEnPassantCapture, Piece capturedPiece) const;

	void makeNormalMove(const moveInfo& move);
	void makeCastlingMove(const moveInfo& move);
	void makeEnPassantMove(const moveInfo& move);
	void makeEnPassantCapture(const moveInfo& move);
	void makePromotionMove(const moveInfo& move);

	void updateCastlingRights(const moveInfo& move);

	void addPieceToBitboards(Square sq, Color clr);
	void removePieceFromBitboards(Square sq, Color clr);

	void updateNonDiagPinStatus(pinInfo& pin, Color clr) const;
	void updateDiagPinStatus(pinInfo& pin, Color clr) const;
	
	bool moveOpensDiscoveredCheck(const moveInfo& move) const;
	bool castlingChecksOpponentKing(const moveInfo& move) const;
	bool enPassantCaptureDiscoveresCheck(const moveInfo& move) const;

	int calculatePstValue(Piece p, Square s) const;
	void updateGameStatus();


	void constructMaterialFEN(std::string& fen) const;
	void constructRightToPlayFEN(std::string& fen) const;
	void constructCastlingRightsFEN(std::string& fen) const;
	void constructEnPassantFileFEN(std::string& fen) const;
	void constructMoveCountFEN(std::string& fen) const;
	
	struct undoMoveInfo {
		Square from;
		Square to;
		Piece movedPiece;
		Piece capturedPiece;
		MoveType moveType;
		int8_t enPassantFile;
		bool whiteLeftCastling;
		bool whiteRightCastling;
		bool blackLeftCastling;
		bool blackRightCastling;
	};

	void undoNormalMove(const undoMoveInfo& move);
	void undoCastlingMove(const undoMoveInfo& move);
	void undoEnPassantMove(const undoMoveInfo& move);
	void undoEnPassantCapture(const undoMoveInfo& move);
	void undoPromotionMove(const undoMoveInfo& move);

	void revertCastlingRights(const undoMoveInfo& move);

	class MoveStack {
		public:
			MoveStack();
			undoMoveInfo* getNextItem();
			const undoMoveInfo* pop();
			bool isEmpty() const;
			uint32_t getSize() const;
		
		private:
			undoMoveInfo _moveStack[MOVE_STACK_CAPACITY];
			uint32_t _stackSize;
	};
	
	struct statePinInfo {
		pinInfo rankPin;
		pinInfo filePin;
		pinInfo diagA1h8Pin;
		pinInfo diagA8h1Pin;
	};




//data members
private:
	Piece _board[8][8];
	
	// Occupation bitboards (4 for each color)
	Bitboard _whitePieces;
	Bitboard _whitePiecesTranspose;
	Bitboard _whitePiecesDiagA1h8;
	Bitboard _whitePiecesDiagA8h1;
	Bitboard _blackPieces;
	Bitboard _blackPiecesTranspose;
	Bitboard _blackPiecesDiagA1h8;
	Bitboard _blackPiecesDiagA8h1;

	// Each memeber of the array shows the number of appropriate 
	// piece available 
	unsigned int _pieceCount[PIECE_NB];

	// Bitboard for each piece where set bits show the
	// positions from which it can attack the king	
	Bitboard _directCheck[PIECE_NB];

	// Complete info of the positions where pinned pieces 
	// can be located and appropriate sliding piece positions
	statePinInfo _statePinInfo;

	const BitboardImpl* _bitboardImpl;
	const ZobKeyImpl* _zobKeyImpl;

	//true - if white's move, false - black's move
	bool _whiteToPlay;
	// the file number of possible enPassant, -1 if none
	int _enPassantFile;
	
	//Shows white and black king position after the move
	Square _whiteKingPosition;
	Square _blackKingPosition;

	// true if appropriate castling is allowed
	bool _whiteLeftCastling;
	bool _whiteRightCastling;
	bool _blackLeftCastling;
	bool _blackRightCastling;

	// true if it is a middle game
	bool _isMiddleGame;

	// Zobrist key for the state of the game
	ZobKey _zobKey;
	
	// Zobrist key for material of the game
	ZobKey _materialZobKey;

	// Piece Square Table value for the state of the game
	int _pstValue;

	// Stack of the moves to be used by undoMove
	MoveStack _moveStack;

	// Halfmove count for fifty move rule
	uint16_t _halfmoveClock;

	// Full move count of the game
	uint16_t _fullmoveCount;

};

}

#endif //POSITIONSTATE_H_
