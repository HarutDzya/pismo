CC = g++
CFLAGS = -Wall -g -c -I. -I../../
LFLAGS = -g
SRC_PATH = ../../

all: Perft.o main.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o MemPool.o
	$(CC) $(LFLAGS) main.o Perft.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o MemPool.o

main.o: main.cpp 
	$(CC) $(CFLAGS) main.cpp -o main.o

Perft.o: Perft.cpp 
	$(CC) $(CFLAGS) Perft.cpp -o Perft.o

PositionState.o: $(SRC_PATH)/PositionState.cpp 
	$(CC) $(CFLAGS) $(SRC_PATH)/PositionState.cpp -o PositionState.o

MoveGenerator.o: $(SRC_PATH)/MoveGenerator.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)/MoveGenerator.cpp -o MoveGenerator.o

BitboardImpl.o: $(SRC_PATH)/BitboardImpl.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)/BitboardImpl.cpp -o BitboardImpl.o

PossibleMoves.o: $(SRC_PATH)/PossibleMoves.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)/PossibleMoves.cpp -o PossibleMoves.o

ZobKeyImpl.o: $(SRC_PATH)/ZobKeyImpl.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)/ZobKeyImpl.cpp -o ZobKeyImpl.o

MemPool.o: $(SRC_PATH)/MemPool.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)/MemPool.cpp -o MemPool.o

clean:
	rm -rf main.o Perft.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o MemPool.o a.out a.exe

