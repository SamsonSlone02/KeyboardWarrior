all: main

main: main.cpp dictionary.h dictionary.cpp
	g++ main.cpp dictionary.h dictionary.cpp libggfonts.a -Wall -okeyboardwarriors -lX11 -lGL -lGLU -lm

clean:
	rm -f keyboardwarriors
