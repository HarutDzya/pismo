CC = g++
#CC = /usr/bin/x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -static -lpthread
CFLAGS = -Wall -O3 -g -std=c++11
LFLAGS = -g

SRCS = PositionState.cpp \
			MoveGenerator.cpp \
			BitboardImpl.cpp \
			MagicMoves.cpp \
			ZobKeyImpl.cpp \
			TranspositionTable.cpp \
			PositionEvaluation.cpp \
			ABCore.cpp \
			MemPool.cpp \
			Uci.cpp \
			main.cpp \
			utils.cpp

OBJS = ${SRCS:.cpp=.o}


all: $(OBJS)
	$(CC) $(OBJS) -o a.out -pthread

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -pthread



clean:
	rm -f *.o a.out

