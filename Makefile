all: lab6

lab6: lab6.cpp dictionary.h dictionary.cpp
	g++ lab6.cpp dictionary.h dictionary.cpp libggfonts.a -Wall -olab6 -lX11 -lGL -lGLU -lm

clean:
	rm -f lab6

