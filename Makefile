CC = g++
CFLAGS = -Wall -O3 -g 
LFLAGS = -g

SRCS = PositionState.cpp \
			MoveGenerator.cpp \
			BitboardImpl.cpp \
			MagicMoves.cpp \
			ZobKeyImpl.cpp \
			TranspositionTable.cpp \
			PositionEvaluation.cpp \
			Core.cpp \
			ABCore.cpp \
			MemPool.cpp \
			main.cpp \
			utils.cpp

OBJS = ${SRCS:.cpp=.o}


all: $(OBJS)
	$(CC) $(OBJS) -o a.out

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@



clean:
	rm -f *.o a.out

