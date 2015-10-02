#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"

namespace pismo
{

class PositionState;
class PossibleMoves;
struct movesArray;

class MoveGenerator
{
public:
	static MoveGenerator* instance();
	void destroy();

	void generateWhiteMoves(const PositionState& pos, movesArray& generatedMoves);
	void generateBlackMoves(const PositionState& pos, movesArray& generatedMoves);

private:
	MoveGenerator();
	~MoveGenerator();
	static MoveGenerator* _instance;

	void generatePawnMoves(Square from, Color clr, const PositionState& pos, movesArray& generatedMoves);
	void generateKnightMoves(Square from, const PositionState& pos, movesArray& generatedMoves);
	void generateKingMoves(Square from, const PositionState& pos, movesArray& generatedMoves);
	void generateRankMoves(Square from, const PositionState& pos, movesArray& generatedMoves);
	void generateFileMoves(Square from, const PositionState& pos, movesArray& generatedMoves);
	void generateDiagA1h8Moves(Square from, const PositionState& pos, movesArray& generatedMoves);
	void generateDiagA8h1Moves(Square from, const PositionState& pos, movesArray& generatedMoves);

	const PossibleMoves* _possibleMoves;
};

}

#endif
