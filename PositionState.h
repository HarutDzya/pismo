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
	bool moveIsLegal(const MoveInfo& move) const;

	/*
	Checks all the rules of the game for legality of the move
	except the issue whther the king is under check after move
	*/
	bool moveIsPseudoLegal(const MoveInfo& move) const;
	
	/*
	Makes a move if the move if legal according to the moveIsLegal
	method
	*/
	void makeMove(const MoveInfo& move);

	/*
	Makes an undo move of the last made move, by reverting all
	state variables to the previous state
	*/
	void undoMove();

	/* Checks to see whether the move checks opponent king
	   by either direct check or discovered check.
	   In order for this function to work the updateDirectCheckArray()
	   and updateDiscoveredChecksInfo() should be invoked first.
	*/
	bool moveChecksOpponentKing(const MoveInfo& move) const;
	
	void updateDirectCheckArray();
	void updateDiscoveredChecksInfo();

	/* Checks to see whether pseudoMove is legal
	   by checking whether the move is not pinned
	   piece move which opens check
	   In order for this function to work the updateStatePinInfo()
	   should be invoked first.
	*/
	bool pseudoMoveIsLegalMove(const MoveInfo& move) const;

	void updateStatePinInfo();	

	/*
	 * returns true if move covers king check
	 * or captures the attacking piece, false otherwise
	 */
	bool isInterposeMove(const MoveInfo& move) const;
	
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

	bool kingUnderCheck() const {return _kingUnderCheck;}

	bool isDoubleCheck() const {return _isDoubleCheck;}

	Square movingKingPosition() const {return _whiteToPlay ? _whiteKingPosition : _blackKingPosition;}

	Bitboard absolutePinsPos() const {return _absolutePinsPos;}

	Bitboard occupiedSquares() const {return _whitePieces | _blackPieces;}
	Bitboard whitePieces() const {return _whitePieces;}
	Bitboard blackPieces() const {return _blackPieces;}

	Bitboard const (&getPiecePos() const)[PIECE_NB] {return _piecePos;}

	Square enPassantTarget() const {return _enPassantFile == -1 ? INVALID_SQUARE : (_whiteToPlay ? (Square) (A6 + _enPassantFile) : (Square) (A3 + _enPassantFile));}  

//private member functions
private:
	void setPiece(Square s, Piece p);
	bool initPositionIsValid(const std::vector<std::pair<Square, Piece> >& pieces) const;
	void initMaterialFEN(const std::string& fen, unsigned int& charCount);
	void initRightToPlayFEN(const std::string& fen, unsigned int& charCount);
	void initCastlingRightsFEN(const std::string& fen, unsigned int& charCount);
	void initEnPassantFileFEN(const std::string& fen, unsigned int& charCount);
	void initMoveCountFEN(const std::string& fen, unsigned int& charCount);
	void updateCheckStatus();

	bool moveIsCastling(const MoveInfo& move) const;
	bool moveIsEnPassantCapture(const MoveInfo& move) const;
	bool pieceIsSlidingPiece(Piece piece) const;

	bool pawnMoveIsLegal(const MoveInfo& move, bool& isEnPassantCapture) const;
	bool knightMoveIsLegal(const MoveInfo& move) const;
	bool bishopMoveIsLegal(const MoveInfo& move) const;
	bool rookMoveIsLegal(const MoveInfo& move) const;
	bool queenMoveIsLegal(const MoveInfo& move) const;
	bool kingMoveIsLegal(const MoveInfo& move) const;
	bool enPassantCaptureIsLegal(const MoveInfo& move) const;
	bool castlingIsLegal(const MoveInfo& move) const;
	bool castlingIsPseudoLegal(const MoveInfo& move) const;
	
	Bitboard squaresUnderAttack(Color attackedColor) const;

	void makeLazyMove(const MoveInfo& move, bool isEnPassantCapture, Piece& capturedPiece) const;
	void undoLazyMove(const MoveInfo& move, bool isEnPassantCapture, Piece capturedPiece) const;

	void makeNormalMove(const MoveInfo& move);
	void makeCaptureMove(const MoveInfo& move);
	void makeCastlingMove(const MoveInfo& move);
	void makeEnPassantMove(const MoveInfo& move);
	void makeEnPassantCapture(const MoveInfo& move);
	void makePromotionMove(const MoveInfo& move);

	void updateCastlingRights(const MoveInfo& move);

	void addPieceToBitboards(Square sq, Piece p, Color clr);
	void removePieceFromBitboards(Square sq, Piece p, Color clr);

	void updateNonDiagPinStatus(PinInfo& pin, Color clr) const;
	void updateDiagPinStatus(PinInfo& pin, Color clr) const;

	void updateMoveChecksOpponentKing(const MoveInfo& move);	
	bool moveOpensDiscoveredCheck(const MoveInfo& move, Square& slidingPiecePos) const;
	bool castlingChecksOpponentKing(const MoveInfo& move, Square& slidingPiecePos) const;
	bool enPassantCaptureDiscoveresCheck(const MoveInfo& move, Square& slidingPiecePos) const;
	bool promotionMoveChecksOpponentKing(const MoveInfo& move) const;

	bool kingPseudoMoveIsLegal(const MoveInfo& move) const;
	bool squareUnderAttack(Square s) const;
	bool pinMoveOpensCheck(const MoveInfo& move) const;
	bool pinEnPassantCaptureOpensCheck(const MoveInfo& move) const;

	int calculatePstValue(Piece p, Square s) const;
	void updateGameStatus();


	void constructMaterialFEN(std::string& fen) const;
	void constructRightToPlayFEN(std::string& fen) const;
	void constructCastlingRightsFEN(std::string& fen) const;
	void constructEnPassantFileFEN(std::string& fen) const;
	void constructMoveCountFEN(std::string& fen) const;
	
	struct UndoMoveInfo {
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

	void undoNormalMove(const UndoMoveInfo& move);
	void undoCaptureMove(const UndoMoveInfo& move);
	void undoCastlingMove(const UndoMoveInfo& move);
	void undoEnPassantMove(const UndoMoveInfo& move);
	void undoEnPassantCapture(const UndoMoveInfo& move);
	void undoPromotionMove(const UndoMoveInfo& move);

	void revertCastlingRights(const UndoMoveInfo& move);

	class MoveStack {
		public:
			MoveStack();
			UndoMoveInfo* getNextItem();
			const UndoMoveInfo* pop();
			bool isEmpty() const;
			uint32_t getSize() const;
		
		private:
			UndoMoveInfo _moveStack[MOVE_STACK_CAPACITY];
			uint32_t _stackSize;
	};
	
	struct StatePinInfo {
		PinInfo rankPin;
		PinInfo filePin;
		PinInfo diagA1h8Pin;
		PinInfo diagA8h1Pin;
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

	// Occupation bitboards for each peace
	Bitboard _piecePos[PIECE_NB];

	// Each memeber of the array shows the number of appropriate 
	// piece available 
	unsigned int _pieceCount[PIECE_NB];

	// Bitboard for each piece where set bits show the
	// positions from which it can attack the king	
	Bitboard _directCheck[PIECE_NB];

	// Complete info of the positions where piece can
    // can be located to open discovered check and 
	// appropirate sliding piece positions
	StatePinInfo _stateDiscCheckInfo;

	// Complete info of the positions where pinned pieces 
	// can be located and appropriate sliding piece positions
	StatePinInfo _statePinInfo;

	const BitboardImpl* _bitboardImpl;
	const ZobKeyImpl* _zobKeyImpl;

	// Bitboard of the positions where the piece should move
	// to stop it's king check, if possible otherwise 0
	Bitboard _absolutePinsPos;

	// Shows whether king is under double check
	// in which case only the king move can save the game
	bool _isDoubleCheck;

	//true - if white's move, false - black's move
	bool _whiteToPlay;
	
	// the file number of possible enPassant, -1 if none
	int _enPassantFile;
	
	//Shows white and black king position after the move
	Square _whiteKingPosition;
	Square _blackKingPosition;

	// Shows whether moving side king is under check
	bool _kingUnderCheck;

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
