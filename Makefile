all: lab6

lab6: lab6.cpp maze.cpp maze.h mazeSolver.cpp mazeSolver.h
	g++ lab6.cpp maze.cpp mazeSolver.cpp libggfonts.a -Wall -olab6 -lX11 -lGL -lGLU -lm

clean:
	rm -f lab6

