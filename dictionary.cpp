#include<fstream>
#include<iostream>
#include<string.h>
#include<iostream>
#include "dictionary.h"
using namespace std;


Dictionary::Dictionary()
{
    fileName = "dictionary.txt";
    wordList = new string[totalWords];
    init_dictionary();
}

void Dictionary::init_dictionary()
{
    ifstream file(fileName);
    string line;
    for(int i = 0; i < totalWords;i++)
    {
        getline(file,line);
        wordList[i] = line;
    }
    cout << "dictionary initialized!" << endl;
}

string Dictionary::getRandomWord()
{
    int rng = (rand() % totalWords) - 1;
    return wordList[rng];
}



