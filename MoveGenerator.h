#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"


namespace pismo
{
enum MoveGenerationStage {
	CAPTURING_MOVES = 0, CHECKING_MOVES,
   	QUITE_MOVES, EVASION_MOVES
}; //TODO: Later add KILLER_MOVES

enum SearchType {
	USUAL_SEARCH = 0,
	EVASION_SEARCH,
	QUITE_SEARCH
};

const unsigned int MAX_AVAILABLE_MOVES = 500;

class PositionState;
class PossibleMoves;
struct MovesArray;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	void generateWhiteMoves(const PositionState& pos, MovesArray& generatedMoves);
	void generateBlackMoves(const PositionState& pos, MovesArray& generatedMoves);

private:
	MoveGenerator();
	~MoveGenerator();
	static MoveGenerator* _instance;

	void prepareMoveGeneration(const PositionState& pos, SearchType type, const MoveInfo& transTableMove);
	void generateAvailableMoves(const PositionState& pos);
	void generatePawnMoves(Square from, const PositionState& pos);
	void generateKnightMoves(Square from, const PositionState& pos);
	void generateKingMoves(Square from, const PositionState& pos);
	void generateRankMoves(Square from, const PositionState& pos);
	void generateFileMoves(Square from, const PositionState& pos);
	void generateDiagA1h8Moves(Square from, const PositionState& pos);
	void generateDiagA8h1Moves(Square from, const PositionState& pos);



	void generatePawnMoves(Square from, Color clr, const PositionState& pos, MovesArray& generatedMoves);
	void generateKnightMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateKingMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateRankMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateFileMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA1h8Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA8h1Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);

	const PossibleMoves* _possibleMoves;
	MovesArray* _availableMoves;
	unsigned int _availableMovesSize;

	MoveGenerationStage _nextStage;
	SearchType _searchType;
	MoveInfo _cachedMove;
};

}

#endif
