#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"


namespace pismo
{
class PositionState;
class PossibleMoves;
class BitboardImpl;
struct MoveGenInfo;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	void prepareMoveGeneration(SearchType type, const MoveInfo& transTableMove, uint16_t depth);
	MoveInfo getTopMove(const PositionState& pos, uint16_t depth);

	// used only for perft testing
	void generatePerftMoves(const PositionState& pos, uint16_t depth);

private:
	MoveGenerator();
	~MoveGenerator();
	MoveGenerator(const MoveGenerator&); //non-copyable
	MoveGenerator& operator=(const MoveGenerator&); //non-assignable

	static MoveGenerator* _instance;

	void generateMovesForUsualSearch();
	void generateMovesForEvasionSearch();
	void generateMovesForQuiescenceSearch();

	// Generates all capturing and promotion moves
	void generateCapturingMoves();

	// Generates all checking moves, which are non-capturing or
	// non-promotion moves
	void generateCheckingMoves();

	// Generates only evasion moves, therefore
	// should be called only if moving side king
	// is under attack
	void generateEvasionMoves();

	// Generates all the moves which are non-promotion,
	// non-capturing
	void generateQuiteMoves();

	void sortCapturingMoves();
	void sortCheckingMoves();
	void sortEvasionMoves();
	void sortQuiteMoves();

	bool equal(const MoveInfo& first, const MoveInfo& second) const;


	void generateKingEvasionMoves();
	void generatePawnsEvasionMoves(Square to, MoveType type);
	void generatePawnsWhiteEvasionMoves(Square to, MoveType type);
	void generatePawnsBlackEvasionMoves(Square to, MoveType type);
	void generateKnightsEvasionMoves(Square to, MoveType type);
	void generateBishopsEvasionMoves(Square to, MoveType type);
	void generateRooksEvasionMoves(Square to, MoveType type);
	void generateQueensEvasionMoves(Square to, MoveType type);

	void generatePawnWhiteCapturingMoves(Square from);
	void generatePawnBlackCapturingMoves(Square from);
	void generateKnightCapturingMoves(Square from);
	void generateRookCapturingMoves(Square from);
	void generateBishopCapturingMoves(Square from);
	void generateQueenCapturingMoves(Square from);
	void generateKingCapturingMoves(Square from);

	void generatePawnDirectCheckingMoves(Square from);
	void generateKnightDirectCheckingMoves(Square from);
	void generateRookDirectCheckingMoves(Square from);
	void generateBishopDirectCheckingMoves(Square from);
	void generateQueenDirectCheckingMoves(Square from);

	void generateWhiteDiscoveredCheckingMoves();
	void generateBlackDiscoveredCheckingMoves();
	void generateDiscoveredCheckingMoves(Square from, Square slidingPiecePos);
	void generatePawnDiscoveredCheckingMoves(Square from, Square slidingPiecePos);
	void generateKnightDiscoveredCheckingMoves(Square from, Square slidingPiecePos);
	void generateRookDiscoveredCheckingMoves(Square from, Square slidingPiecePos);
	void generateBishopDiscoveredCheckingMoves(Square from, Square slidingPiecePos);
	void generateKingDiscoveredCheckingMoves(Square from, Square slidingPiecePos);

	void generatePawnWhiteQuiteMoves(Square from);
	void generatePawnBlackQuiteMoves(Square from);
	void generateKnightQuiteMoves(Square from);
	void generateRookQuiteMoves(Square from);
	void generateBishopQuiteMoves(Square from);
	void generateQueenQuiteMoves(Square from);
	void generateKingWhiteQuiteMoves(Square from);
	void generateKingBlackQuiteMoves(Square from);

	const BitboardImpl* _bitboardImpl;
	const PositionState* _positionState;
	MoveGenInfo* _moveGenInfo;
};

}

#endif
