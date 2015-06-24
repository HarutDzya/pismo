all: PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o main.o
	g++ -g main.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

MoveGenerator.o: MoveGenerator.cpp
	g++ -g -c MoveGenerator.cpp -o MoveGenerator.o

BitboardImpl.o: BitboardImpl.cpp
	g++ -g -c BitboardImpl.cpp -o BitboardImpl.o

PossibleMoves.o: PossibleMoves.cpp
	g++ -g -c PossibleMoves.cpp -o PossibleMoves.o

ZobKeyImpl.o: ZobKeyImpl.cpp
	g++ -g -c ZobKeyImpl.cpp -o ZobKeyImpl.o

TranspositonTable.o: TranspositionTable.cpp
	g++ -g -c TranspositionTable.cpp -o TranspositionTable.o

main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o a.out a.exe
