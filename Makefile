run: PositionState.o BitboardBase.o main.o
	g++ -g main.o PositionState.o BitboardBase.o 

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

BitboardBase.o: BitboardBase.cpp
	g++ -g -c BitboardBase.cpp -o BitboardBase.o

main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  BitboardBase.o a.out
