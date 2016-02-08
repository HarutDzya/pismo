CC = g++
CFLAGS = -Wall -O3 -g -I../../../
LFLAGS = -g

SRCS = ../../../PositionState.cpp \
			../../../MoveGenerator.cpp \
			../../../BitboardImpl.cpp \
			../../../MagicMoves.cpp \
			../../../ZobKeyImpl.cpp \
			../../../TranspositionTable.cpp \
			../../../PositionEvaluation.cpp \
			../../../MemPool.cpp \
			../../../utils.cpp \
			main.cpp

OBJS = ${SRCS:.cpp=.o}

all: $(OBJS)
	$(CC) $(OBJS) -o pismo

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f ../../../*.o pismo
