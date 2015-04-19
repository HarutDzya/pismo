run: PositionState.o PositionStateMove.o main.o
	g++ -g main.o PositionState.o PositionStateMove.o

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

PositionStateMove.o: PositionStateMove.cpp 
	g++ -g -c PositionStateMove.cpp -o PositionStateMove.o


main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o PositionStateMove.o a.out
