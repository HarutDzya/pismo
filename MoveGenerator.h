#ifndef MOVEGENERATOR_H_
#define MOVEGENERATOR_H_

#include "utils.h"

namespace pismo
{

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

	void generatePawnMoves(Square from, Color clr, const PositionState& pos, MovesArray& generatedMoves);
	void generateKnightMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateKingMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateRankMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateFileMoves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA1h8Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);
	void generateDiagA8h1Moves(Square from, const PositionState& pos, MovesArray& generatedMoves);

	const PossibleMoves* _possibleMoves;
};

}

#endif
