
#include<stdio.h>
#include<iostream>
#include "maze.h"

#define MAX_STEPS 500

void mazeSolver(char maze[][(WIDTH * 2) + 1], int height, int width, char * steps, int &nSteps)
{   
    nSteps = 0;
    
    //find start/finish;
    int startPos[2] = {-1,-1};
    int finishPos[2] = {-1,-1};

    for(int i = 0; i < height;i++)
    {
        for(int j = 0; j < width; j++)
        {
            if(maze[i][j] == 's')
            {
                startPos[0] = j;
                startPos[1] = j;
            }
            if(maze[i][j] == 'f')
            {
                finishPos[0] = i;
                finishPos[1] = i;
            }
        }
    }

    //check that start and end was found
    bool unfinishedMaze = false;
    if(startPos[0] == -1 || startPos[1] == -1)
    {
        printf("start not found\n");
        unfinishedMaze = true;
    }
    if(finishPos[0] == -1 || finishPos[1] == -1)
    {
        printf("finish not found\n");
        unfinishedMaze = true;
    }
    if(unfinishedMaze)
    {
        printf("maze is unfinished, not solving and returning\n");       
        return;
    }

    for(int i = 0;i < height;i++)
    {
        for(int j = 0; j < width;j++)
        {
            printf("%c",maze[i][j]);
        }
        printf("\n");
    }
    fflush(stdout);


    int pos[2] = {startPos[0],startPos[1]};

    //nesw
    char dir[4]= {' ',' ',' ',' '};
    char sdir[4] = {'n','e','s','w'};
    int ndir = 0;
    int count = 0;
    char currentDir;
    for(int i = 0; i < MAX_STEPS;i++)
    {

        if(maze[pos[0] + 1][pos[1]] != '#')
        {
            dir[ndir] = sdir[2];
            //pos[0]+=2;
            ndir++;
        }
        if(maze[pos[0] - 1][pos[1]] != '#')
        {   
            dir[ndir] = sdir[0];
            //pos[0]-=2;
            ndir++;
        }
        if(maze[pos[0]][pos[1] + 1] != '#')
        { 
            dir[ndir] = sdir[1];
            // pos[1]+=2;
            ndir++;
        }
        if(maze[pos[0]][pos[1] - 1] != '#')
        {
            dir[ndir] = sdir[3];
            //pos[1]-=2;
            ndir++;
        }
        bool canGoStraight = false;
        for(int i = 0; i < ndir;i++)
        {
            if(currentDir == dir[i] && ndir == 2)
            {
                canGoStraight = true;
                if(currentDir == 'n')
                {
                pos[0]-=2;
                steps[count] = 'n';
               

                }
                if(currentDir == 's')
                {
                    pos[0]+=2;
                steps[count] = 's';
                }
                if(currentDir == 'e')
                {
                    pos[1]+=2;
                steps[count] = 'e';
                }
                if(currentDir == 'w')
                {
                    pos[1]-=2;
                steps[count] = 'w';
                }
            count++;
            nSteps++;

            }
        }

        if(!canGoStraight)
        {
            int rdir = rand() % ndir;
            steps[count] =  dir[rdir];

            currentDir = dir[rdir];
            switch(currentDir)
            {
                case 'w':
                    pos[1]-=2;
                    break;
                case 'e':
                    pos[1]+=2;
                    break;
                case 'n':
                    pos[0]-=2;
                    break;
                case 's':
                    pos[0]+=2;
                    break;
            }
        count++;
        nSteps++;
        }

        ndir = 0;
        dir[0] = ' ';
        dir[1] = ' ';
        dir[2] = ' ';
        dir[3] = ' ';


    }




}


