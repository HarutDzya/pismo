run: Position.o main.o
	g++ -g main.o Position.o

Position.o: Position.cpp Position.h utils.h
	g++ -g -c Position.cpp -o Position.o

main.o: main.cpp Position.h utils.h
	g++ -g -c main.cpp -o main.o

clean:
	rm -rf main.o Position.o a.out
