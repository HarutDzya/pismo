all: PositionState.o MoveGenerator.o BitboardImpl.o MovePosImpl.o main.o
	g++ -g main.o PositionState.o MoveGenerator.o BitboardImpl.o MovePosImpl.o

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

MoveGenerator.o: MoveGenerator.cpp
	g++ -g -c MoveGenerator.cpp -o MoveGenerator.o

BitboardImpl.o: BitboardImpl.cpp
	g++ -g -c BitboardImpl.cpp -o BitboardImpl.o

MovePosImpl.o: MovePosImpl.cpp
	g++ -g -c MovePosImpl.cpp -o MovePosImpl.o

main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  MoveGenerator.o BitboardImpl.o MovePosImpl.o a.out a.exe
