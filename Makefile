CC = g++
CFLAGS = -Wall -g -c
LFLAGS = -g

all: PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o PositionEvaluation.o Core.o MemPool.o main.o
	$(CC) $(LFLAGS) main.o PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o PositionEvaluation.o Core.o MemPool.o

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

TranspositionTable.o: TranspositionTable.cpp
	$(CC) $(CFLAGS) TranspositionTable.cpp -o TranspositionTable.o

PositionEvaluation.o: PositionEvaluation.cpp
	$(CC) $(CFLAGS) PositionEvaluation.cpp -o PositionEvaluation.o

Core.o: Core.cpp
	$(CC) $(CFLAGS) Core.cpp -o Core.o

MemPool.o: MemPool.cpp
	$(CC) $(CFLAGS) MemPool.cpp -o MemPool.o

main.o: main.cpp 
	$(CC) $(CFLAGS) main.cpp -o main.o

clean:
	rm -rf main.o PositionState.o  MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o PositionEvaluation.o Core.o MemPool.o a.out a.exe
