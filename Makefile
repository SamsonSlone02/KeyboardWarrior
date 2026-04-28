all: main

main: main.cpp dictionary.cpp dictionary.h GameSound.cpp
	g++ main.cpp GameSound.cpp dictionary.cpp libggfonts.a -Wall -okeyboardwarriors -I/usr/include/AL -lalut -lopenal -lX11 -lGL -lGLU -lm

clean:
	rm -f keyboardwarriors
