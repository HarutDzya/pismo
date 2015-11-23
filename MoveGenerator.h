#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"


namespace pismo
{
enum MoveGenerationStage {
	CAPTURING_MOVES = 0, CHECKING_MOVES,
   	QUITE_MOVES, EVASION_MOVES, SEARCH_FINISHED
}; //TODO: Later add KILLER_MOVES

enum SearchType {
	USUAL_SEARCH = 0,
	EVASION_SEARCH,
	QUITE_SEARCH
};

const unsigned int MAX_AVAILABLE_MOVES = 500;

class PositionState;
class PossibleMoves;
class BitboardImpl;
struct MovesArray;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	void prepareMoveGeneration(const PositionState& pos, SearchType type, const MoveInfo& transTableMove, MovesArray& generatedMoves);
	MoveInfo getTopMove();

	void generateWhiteMoves(const PositionState& pos, MovesArray& generatedMoves);
	void generateBlackMoves(const PositionState& pos, MovesArray& generatedMoves);

private:
	MoveGenerator();
	~MoveGenerator();
	MoveGenerator(const MoveGenerator&); //non-copyable
	MoveGenerator& operator=(const MoveGenerator&); //non-assignable

	static MoveGenerator* _instance;

	void generateMovesForUsualSearch();
	void generateMovesForEvasionSearch();
	void generateMovesForQuiteSearch();

	void generateCapturingMoves();
	void generateCheckingMoves();
	void generateEvasionMoves();
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

	void generatePawnMoves(Square from, Color clr, const PositionState& pos, MovesArray& generatedMoves);
	void generateKnightMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateKingMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateRankMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateFileMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA1h8Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA8h1Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);

	const PossibleMoves* _possibleMoves;
	const BitboardImpl* _bitboardImpl;
	const PositionState* _positionState;
	MovesArray* _availableMoves;
	unsigned int _currentMovePos;
	unsigned int _availableMovesSize;

	MoveGenerationStage _nextStage;
	SearchType _searchType;
	MoveInfo _cachedMove;
};

}

#endif
