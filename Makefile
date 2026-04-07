all: main

main: main.cpp dictionary.cpp dictionary.h
	g++ main.cpp dictionary.cpp libggfonts.a -Wall -okeyboardwarriors -lX11 -lGL -lGLU -lm

clean:
	rm -f keyboardwarriors
