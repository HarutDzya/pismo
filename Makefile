all: PositionState.o BitboardImpl.o main.o
	g++ -g main.o PositionState.o BitboardImpl.o 

PositionState.o: PositionState.cpp 
	g++ -g -c PositionState.cpp -o PositionState.o

BitboardImpl.o: BitboardImpl.cpp
	g++ -g -c BitboardImpl.cpp -o BitboardImpl.o

main.o: main.cpp 
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  BitboardImpl.o a.out
