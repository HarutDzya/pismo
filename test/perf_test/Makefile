CC = g++
CFLAGS = -Wall -g -c -I. -I../../
LFLAGS = -g
OBJS = $(patsubst %.o,../../%.o, PositionState.o MoveGenerator.o BitboardImpl.o PossibleMoves.o ZobKeyImpl.o TranspositionTable.o PositionEvaluation.o Core.o)
ISMALL = positions1.txt positions2.txt output_small.txt
IBIG = positions10.txt positions11.txt output_big.txt

all: Perft.o main.o
	$(CC) $(LFLAGS) main.o Perft.o $(OBJS)


Perft.o: Perft.cpp 
	$(CC) $(CFLAGS) Perft.cpp -o Perft.o

main.o: main.cpp 
	$(CC) $(CFLAGS) main.cpp -o main.o

srun:
	./a.exe $(ISMALL)

brun:
	./a.exe $(IBIG)

clean:
	rm -rf main.o Perft.o a.out a.exe
