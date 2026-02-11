//personal project created by Samson Slone
#include<iostream>
#include<random>
#include<map>
#include<stack>
#include<algorithm>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <unistd.h>


#define HEIGHT 10
#define WIDTH 10

using namespace std;
int myrandom (int i) { return rand()%i;}
class Cell 
{
    protected:
        int x,y;
        bool isVisited;
        map<char,bool> walls = {{'N',true},{'S',true},{'E',true},{'W',true}};
    public:
        
        Cell(int in_x = 0, int in_y = 0)
        {
            isVisited = false;
           
            x = in_x;
            y = in_y;


        }
        ~Cell()
        {
           
        }
    
        void setVisited() //sets cell as visited
        {
            isVisited = true;
        }
        void setWall(char dir) //breaks wall dir N, S, E, W
        {
            walls[dir] = false;
            if(!walls[dir])
            {
                //     cout << dir << " is false" << endl;
            }
            //else cout << "didnt set" << endl;
        }
        int getX()
        {
            return x;
        }
        int getY() // return y-value
        {
            return y;
        }
        bool getVisited() // return if cell was visited
        {
            return isVisited;
        }
        string getDisplay1()    //display 1st line of cell
        {
            string temp;
            if(!walls['N'])
            {
                temp = "#   #";
            }else{
                temp ="#####";
            }
            return temp;
        }      
        string getDisplay2()     //display 2nd line of cell
        {
            string temp;
            if(!walls['W'])
            {
                temp += " ";
            }else{
                temp += "#";
            }

            if(isVisited)
            {temp += "   ";
            }
            else temp +="###";
            if(!walls['E'])
            {
                temp += " ";
            }else{
                temp += "#";
            }
            return temp;
        }   
        string getDisplay3()  // displat 3rd line of cell
        {
            string temp;
            if(!walls['S'])
            {

                temp = "#   #";
            }else{
                temp = "#####";
            }
            return temp;
        }   


};

class Maze
{
    protected:

    public:
        Maze(){}

        void printMaze(Cell myMaze[HEIGHT][WIDTH])
        {
            string printTemp;

            for(int j = 0;j < HEIGHT; j++)
            {
                for(int i = 0; i < WIDTH; i++)
                {


                    printTemp += myMaze[j][i].getDisplay1();


                }
                printTemp += "\n";
                for(int i = 0; i < WIDTH; i++)
                {

                    printTemp +=myMaze[j][i].getDisplay2();

                }
                printTemp += "\n";
                for(int i = 0; i < WIDTH; i++)
                {

                    printTemp += myMaze[j][i].getDisplay3();

                }
                printTemp += "\n";
            }


            system("clear");

            printf("%s",printTemp.c_str());


        }

        void createMaze()
        {
        
            srand ( unsigned ( std::time(0) ) );
            bool showProcess;
            char ans;
            cout << "Show process?(Y/N): ";     
            cin >> ans;
            if(ans == 'y' || ans =='Y')
            {
                showProcess = true;
            }
            else showProcess = false;


            Cell myMaze[HEIGHT][WIDTH];
            for(int i =0;i < HEIGHT; i++)  //initialize maze matrix
            {
                for(int j = 0;j< WIDTH; j++)
                {
                    Cell * node = new Cell(j,i);

                    myMaze[i][j] = * node;

                }



            }
            printMaze(myMaze);
            int x = 0;
            int y = 0;

            stack<Cell *> mystack;
            myMaze[0][0].setVisited();
            mystack.push(&(myMaze[y][x]));
            Cell * current;
            Cell *arr2[2];
            Cell *arr3[3];
            Cell *arr4[4];
            int tempX,tempY;
            string printTemp;


            while(mystack.size() > 0)
            {
                printTemp = "";

                cout << mystack.size() << endl;

                if(showProcess)
                {
                    printMaze(myMaze);
                }

                current = mystack.top();


                mystack.pop();


                x = current->getX();
                y = current->getY();



                if(x == 0 && y==0) //if top left
                {
                    if(!myMaze[y][x+1].getVisited() || !myMaze[y+1][x].getVisited())
                    {
                        mystack.push(current);
                        arr2[0] = &(myMaze[y][x+1]); 
                        arr2[1] = &(myMaze[y+1][x]);
                        random_shuffle(&arr2[0], &arr2[2], myrandom);
                        for(int i  = 0; i < 2;i++)
                        {
                            if(!arr2[i]->getVisited())
                            { 
                                tempX = arr2[i]->getX()- current->getX() ;
                                tempY = arr2[i]->getY() - current->getY();

                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr2[i]->setWall('W');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    break;
                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr2[i]->setWall('N');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    break;
                                }
                            }
                        }
                    }

                }else if(x == WIDTH - 1 && y==0) //if top right
                {
                    if(!myMaze[y][x-1].getVisited() || !myMaze[y+1][x].getVisited())
                    {
                        mystack.push(current);
                        arr2[0] = &myMaze[y][x-1]; 
                        arr2[1] = &myMaze[y+1][x];
                        random_shuffle(&arr2[0], &arr2[2], myrandom);
                        for(int i  = 0; i < 2;i++)
                        {
                            if(!arr2[i]->getVisited())
                            {
                                tempX = arr2[i]->getX()- current->getX() ;
                                tempY = arr2[i]->getY() - current->getY();

                                if(tempX == -1)
                                {
                                    current->setWall('W');
                                    arr2[i]->setWall('E');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    // cout << "push" << endl;
                                    break;

                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr2[i]->setWall('N');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    // cout << "push" << endl;
                                    break;
                                }

                            }
                            //cout<< "no possible areas" << endl;
                        }

                    }
                }else if(x == 0 && y== HEIGHT - 1) // is bottom left
                {
                    if(!myMaze[y][x+1].getVisited() || !myMaze[y-1][x].getVisited())
                    {
                        mystack.push(current);
                        arr2[0] = &myMaze[y][x+1]; 
                        arr2[1] = &myMaze[y-1][x];

                        random_shuffle(&arr2[0], &arr2[2], myrandom);
                        for(int i  = 0; i < 2;i++)
                        {
                            if(!arr2[i]->getVisited())
                            {
                                tempX = arr2[i]->getX()- current->getX() ;
                                tempY = arr2[i]->getY() - current->getY();

                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr2[i]->setWall('W');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    break;
                                }
                                if(tempY == -1)
                                {
                                    current->setWall('N');
                                    arr2[i]->setWall('S');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    break;
                                }
                            }

                        }

                    }
                }else if(x == WIDTH - 1 && y== HEIGHT - 1) // if bottom right
                {
                    if(!myMaze[y][x-1].getVisited() || !myMaze[y-1][x].getVisited())
                    {

                        mystack.push(current);
                        arr2[0] = &myMaze[y][x-1]; 
                        arr2[1] = &myMaze[y-1][x];
                        random_shuffle(&arr2[0], &arr2[2], myrandom);
                        for(int i  = 0; i < 2;i++)
                        {

                            if(!arr2[i]->getVisited())
                            {

                                tempX = arr2[i]->getX()- current->getX() ;
                                tempY = arr2[i]->getY() - current->getY();

                                if(tempX == -1)
                                {

                                    current->setWall('W');
                                    arr2[i]->setWall('E');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];
                                    break;
                                }
                                if(tempY == -1)
                                {

                                    current->setWall('N');
                                    arr2[i]->setWall('S');
                                    arr2[i]->setVisited();
                                    mystack.push(arr2[i]);
                                    current = arr2[i];                            
                                    break;
                                }   
                            }
                        }
                    }
                }else if(x == WIDTH -1) // if against right wall
                {
                    if(!myMaze[y][x-1].getVisited() || !myMaze[y-1][x].getVisited() || !myMaze[y+1][x].getVisited())
                    {
                        mystack.push(current);
                        arr3[0] = &myMaze[y][x-1]; 
                        arr3[1] = &myMaze[y-1][x];
                        arr3[2] = &myMaze[y+1][x];
                        random_shuffle(&arr3[0], &arr3[3], myrandom);
                        for(int i  = 0; i < 3;i++)
                        {
                            if(!arr3[i]->getVisited())
                            {
                                tempX =  arr3[i]->getX() - current->getX();
                                tempY =  arr3[i]->getY() - current->getY();

                                if(tempX == -1)
                                {
                                    current->setWall('W');
                                    arr3[i]->setWall('E');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    break;
                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr3[i]->setWall('N');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    break;
                                }
                                if(tempY == -1)
                                {
                                    current->setWall('N');
                                    arr3[i]->setWall('S');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    break;
                                }
                            }
                        }
                    }
                }else if(x == 0) // if against left wall
                {

                    //cout << "X IS AGAINST LEFT WALL ________________" << endl;
                    if(!myMaze[y][x+1].getVisited() || !myMaze[y-1][x].getVisited() || !myMaze[y+1][x].getVisited())
                    {
                        //cout << "still has unvisited neighbors" << endl;
                        mystack.push(current);
                        arr3[0] = &myMaze[y][x+1]; 
                        arr3[1] = &myMaze[y-1][x];
                        arr3[2] = &myMaze[y+1][x];

                        random_shuffle(&arr3[0], &arr3[3], myrandom);
                        for(int i  = 0; i < 3;i++)
                        {   
                            if(!arr3[i]->getVisited())
                            {
                                //cout << "testing against " << arr3[i]->getX() << ", " << arr3[i]->getY() << endl;
                                tempX =  arr3[i]->getX() - current->getX();
                                tempY =  arr3[i]->getY() - current->getY();

                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr3[i]->setWall('W');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr3[i]->setWall('N');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempY== -1)
                                {
                                    current->setWall('N');
                                    arr3[i]->setWall('S');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }

                            }
                            //cout<< "no possible areas" << endl;
                        }

                    }




                }else if(y == HEIGHT -1) // if against bottom
                {
                    if(!myMaze[y][x+1].getVisited() || !myMaze[y][x-1].getVisited()|| !myMaze[y-1][x].getVisited())
                    {
                        mystack.push(current);
                        arr3[0] = &myMaze[y][x+1]; 
                        arr3[1] = &myMaze[y][x-1];
                        arr3[2] = &myMaze[y-1][x];
                        random_shuffle(&arr3[0], &arr3[3], myrandom);
                        for(int i  = 0; i < 3;i++)
                        {   
                            if(!arr3[i]->getVisited())
                            {
                                tempX =  arr3[i]->getX() - current->getX();
                                tempY =  arr3[i]->getY() - current->getY();

                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr3[i]->setWall('W');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempY == -1)
                                {
                                    current->setWall('N');
                                    arr3[i]->setWall('S');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempX == -1)
                                {
                                    current->setWall('W');
                                    arr3[i]->setWall('E');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }

                            }
                            //cout<< "no possible areas" << endl;
                        }
                    }
                }else if(y == 0) // if against top
                {
                    if(!myMaze[y][x+1].getVisited() || !myMaze[y][x-1].getVisited() || !myMaze[y+1][x].getVisited())
                    {
                        //cout<< "triggered" << endl;
                        mystack.push(current);
                        arr3[0] = &myMaze[y][x+1]; 
                        arr3[1] = &myMaze[y][x-1];
                        arr3[2] = &myMaze[y+1][x];

                        random_shuffle(&arr3[0], &arr3[3],myrandom);
                        //  //cout << arr3[0].getX() << ", " << arr3[0].getY() << "shuffled array"<< endl;
                        for(int i  = 0; i < 3;i++)
                        {   
                            if(!arr3[i]->getVisited())
                            {
                                tempX =  arr3[i]->getX() - current->getX();
                                tempY =  arr3[i]->getY() - current->getY();
                                //cout << tempY << endl;


                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr3[i]->setWall('W');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];

                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr3[i]->setWall('N');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempX == -1)
                                {
                                    current->setWall('W');
                                    arr3[i]->setWall('E');
                                    arr3[i]->setVisited();
                                    mystack.push(arr3[i]);
                                    current = arr3[i];
                                    //cout << "push" << endl;
                                    break;
                                }

                            }
                            //cout<< "no possible areas" << endl;
                        }
                    }
                }else
                {

                    if(!myMaze[y][x+1].getVisited() || !myMaze[y][x-1].getVisited() || !myMaze[y+1][x].getVisited() || !myMaze[y-1][x].getVisited())
                    {
                        mystack.push(current);
                        arr4[0] = &myMaze[y][x+1]; 
                        arr4[1] = &myMaze[y][x-1];
                        arr4[2] = &myMaze[y+1][x];
                        arr4[3] = &myMaze[y-1][x];
                        random_shuffle(&arr4[0], &arr4[4],myrandom);
                        for(int i  = 0; i < 4;i++)
                        {   
                            if(!arr4[i]->getVisited())
                            {
                                tempX =  arr4[i]->getX() - current->getX();
                                tempY =  arr4[i]->getY() - current->getY();
                                //cout << tempY << endl;


                                if(tempX == 1)
                                {
                                    current->setWall('E');
                                    arr4[i]->setWall('W');
                                    arr4[i]->setVisited();
                                    mystack.push(arr4[i]);
                                    current = arr4[i];
                                    //cout << "push" << endl;
                                    break;
                                }
                                if(tempY == 1)
                                {
                                    current->setWall('S');
                                    arr4[i]->setWall('N');
                                    arr4[i]->setVisited();
                                    mystack.push(arr4[i]);
                                    current = arr4[i];
                                    break;
                                }
                                if(tempY == -1)
                                {
                                    current->setWall('N');
                                    arr4[i]->setWall('S');
                                    arr4[i]->setVisited();
                                    mystack.push(arr4[i]);
                                    current = arr4[i];
                                    break;
                                }
                                if(tempX == -1)
                                {
                                    current->setWall('W');
                                    arr4[i]->setWall('E');
                                    arr4[i]->setVisited();
                                    mystack.push(arr4[i]);
                                    current = arr4[i];
                                    break;
                                }

                            }

                        }
                    }
                }
            }

        system("clear");
        printMaze(myMaze);

        }

};




int main()
{
    //print final Maze Soulution
    Maze myMaze;
    myMaze.createMaze();
}
