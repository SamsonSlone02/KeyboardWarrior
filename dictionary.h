//dictionary.h
#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <string>
using namespace std;
class Dictionary
{
    private:
        const int totalWords = 27556;
        string fileName;
        string * wordList;

    public:
        Dictionary();
        void init_dictionary();
        string getRandomWord();

};

#endif
