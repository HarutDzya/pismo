all: PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o main.o
	g++ -g main.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

MoveGenerator.o: MoveGenerator.cpp
	g++ -g -c MoveGenerator.cpp -o MoveGenerator.o

BitboardImpl.o: BitboardImpl.cpp
	g++ -g -c BitboardImpl.cpp -o BitboardImpl.o

PossibleMoves.o: PossibleMoves.cpp
	g++ -g -c PossibleMoves.cpp -o PossibleMoves.o

main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  MoveGenerator.o BitboardImpl.o PossibleMoves.o a.out a.exe
