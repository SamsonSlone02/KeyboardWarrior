#ifndef MAZE_H
#define MAZE_H



//personal project created by Samson Slone
#include<iostream>
#include<random>
#include<map>
#include<stack>
#include<algorithm>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <unistd.h>


#define HEIGHT 15
#define WIDTH 15

using namespace std;
int myrandom (int i);
class Cell 
{
    protected:
        int x=0,y=0;
        bool isVisited = false;
        bool isEnd = false;
        bool isStart = false;
        map<char,bool> walls = {{'N',true},{'S',true},{'E',true},{'W',true}};    
    public:

        Cell(int in_x, int in_y);
        ~Cell();
        void setVisited(); //sets cell as visited
        void setEnd();
        void setStart();
        void setWall(char dir); //breaks wall dir N, S, E, W
        int getX();
        int getY(); // return y-value
        bool getVisited(); // return if cell was visited
        bool getStart();
        bool getEnd(); 
        string getDisplay1();    //display 1st line of cell
        string getDisplay2();     //display 2nd line of cell
        string getDisplay3();  // displat 3rd line of cell


};

class Maze
{
    protected:
    public:
        int gridw = WIDTH;
        int gridh = HEIGHT;

        int adj_gridw = (WIDTH * 2) + 1;
        int adj_gridh = (HEIGHT * 2) + 1;

        Cell myMaze[HEIGHT][WIDTH];
        Cell *arr2[2];
        Cell *arr3[3];
        Cell *arr4[4];
        char mazeOutput[(HEIGHT * 2)+1][(WIDTH * 2) + 1];
        Maze();
        void printMaze(Cell in_Maze[HEIGHT][WIDTH]);
        void createMaze();
};

#endif //MAZE_H


