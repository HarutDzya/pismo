CC = g++
CFLAGS = -Wall -O3 -g -I../../
LFLAGS = -g

SRCS = ../../PositionState.cpp \
			../../MoveGenerator.cpp \
			../../BitboardImpl.cpp \
			../../MagicMoves.cpp \
			../../ZobKeyImpl.cpp \
			../../TranspositionTable.cpp \
			../../PositionEvaluation.cpp \
			../../ABCore.cpp \
			../../MemPool.cpp \
			main.cpp \
			Perft.cpp \
			../../utils.cpp

OBJS = ${SRCS:.cpp=.o}

all: $(OBJS)
	$(CC) $(OBJS) -o perft

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o perft
