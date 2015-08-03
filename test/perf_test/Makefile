CC = g++
CFLAGS = -Wall -g -c
LFLAGS = -g

all: Perft.o main.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o
	$(CC) $(LFLAGS) main.o Perft.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o

main.o: main.cpp 
	$(CC) $(CFLAGS) main.cpp -o main.o

Perft.o: Perft.cpp 
	$(CC) $(CFLAGS) Perft.cpp -o Perft.o

PositionState.o: PositionState.cpp 
	$(CC) $(CFLAGS) PositionState.cpp -o PositionState.o

MoveGenerator.o: MoveGenerator.cpp
	$(CC) $(CFLAGS) MoveGenerator.cpp -o MoveGenerator.o

BitboardImpl.o: BitboardImpl.cpp
	$(CC) $(CFLAGS) BitboardImpl.cpp -o BitboardImpl.o

PossibleMoves.o: PossibleMoves.cpp
	$(CC) $(CFLAGS) PossibleMoves.cpp -o PossibleMoves.o

ZobKeyImpl.o: ZobKeyImpl.cpp
	$(CC) $(CFLAGS) ZobKeyImpl.cpp -o ZobKeyImpl.o

clean:
	rm -rf main.o Perft.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o a.out a.exe

