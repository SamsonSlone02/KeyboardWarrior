//
//
//
//
//author:  Gordon Griesel
//OpenGL
//lab-6 starting framework
#include <cctype>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"
#include <stack>
#include <vector>
#include <filesystem>
#include "GameSound.h"


#include "dictionary.h"
using namespace std;

typedef float Flt;
typedef Flt Vec[3];
void vecMake(Flt a, Flt b, Flt c, Vec v);
#define rnd() (Flt)rand() / (Flt)RAND_MAX
#define PI 3.14159265358979323846264338327950

//setup timers -------------------------------------------------
struct timespec res;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double renderCountdown=0.0;
double timeSpan=0.0;
double statsCountdown=0.0;
const double physicsRate = 1.0 / 30.0;
const double renderRate = 1.0 / 60.0;
const double maxFrameTime = 0.25;
const double oobillion = 1.0 / 1e9;
int physicsFrames = 0;
int renderFrames = 0;
int displayedUPS = 0;
int displayedFPS = 0;
double timeDiff(struct timespec *start, struct timespec *end) {
	return (double)(end->tv_sec - start->tv_sec ) +
			(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
	memcpy(dest, source, sizeof(struct timespec));
}
//end of timers -----------------------------------------------

class PlayerData
{

};




stack<char> currentText;

class Image {
public:
        int width, height, max;
        char *data;
        Image() { }
        Image(const char *fname) {
                bool isPPM = true;
                char str[1200];
                char newfile[200];
                ifstream fin;
                char *p = strstr((char *)fname, ".ppm");
                if (!p) {
                        //not a ppm file
                        isPPM = false;
                        strcpy(newfile, fname);
                        newfile[strlen(newfile)-4] = '\0';
                        strcat(newfile, ".ppm");
                        sprintf(str, "convert %s %s", fname, newfile);
                        system(str);
                        fin.open(newfile);
                } else {
                        fin.open(fname);
                }
                char p6[10];
                fin >> p6;
                fin >> width >> height;
                fin >> max;
                data = new char [width * height * 3];
                fin.read(data, 1);
                fin.read(data, width * height * 3);
                fin.close();
                if (!isPPM)
                        unlink(newfile);
        }

        Image(const std::string& fname) {
                const char* cstr = fname.c_str();
                bool isPPM = true;
                char str[1200];
                char newfile[200];
                ifstream fin;
                char *p = strstr((char *)cstr, ".ppm");
                if (!p) {
                        //not a ppm file
                        isPPM = false;
                        strcpy(newfile, cstr);
                        newfile[strlen(newfile)-4] = '\0';
                        strcat(newfile, ".ppm");
                        sprintf(str, "convert %s %s", cstr, newfile);
                        system(str);
                        fin.open(newfile);
                } else {
                        fin.open(cstr);
                }
                char p6[10];
                fin >> p6;
                fin >> width >> height;
                fin >> max;
                data = new char [width * height * 3];
                fin.read(data, 1);
                fin.read(data, width * height * 3);
                fin.close();
                if (!isPPM)
                        unlink(newfile);
        }

}; Image img[7] = {"wall.png","carpet.png","sky4.jpg","ascii.png","selfie_cat.png","ceiling.png","weapon.png"};
//Image enemies[] = {"enemy.png", "enemy2.png"};
//const int enemyTextureCount = sizeof(enemies) / sizeof(enemies[0]);

std::vector<Image> get_files_to_array(const std::string& path) {
    std::vector<Image> file_list;

    try {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                // filter for files only, skipping sub-directories
                if (std::filesystem::is_regular_file(entry)) {
                    file_list.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& err) {
        std::cerr << "Filesystem error: " << err.what() << std::endl;
    }

    return file_list;
}

std::vector<Image> enemies = get_files_to_array("./enemies");



class Texture {
    public:
        Image *backImage;
        GLuint backTexture;
        float xc[2];
        float yc[2];
};

class Global {
    public:
        int xres, yres;
        GLfloat lightAmbient[4];
        GLfloat lightDiffuse[4];
        GLfloat lightSpecular[4];
        GLfloat lightPosition[4];
        int lesson_num;
        Flt rtri;
        Flt rquad;
        Flt cubeRot[3];
        Flt cubeAng[3];
        Texture tex;
        Texture carpetTex;
        Texture skyTex;
        Texture asciiTex;
        Texture *enemyTex = new Texture[enemies.size()];
        Texture weaponTex;
        Texture ceilingTex;
        int cx;
        int cy;
        int cz;
        int pause;
        GameSound sound;

        int keys[65536];

        Flt yaw;
        Flt pitch;
        Vec cameraUp;
        Vec cameraPos;
        Vec cameraFront;


        int mazeHeight;
        int mazeWidth;

        Vec playerTileLocation;
        

        char targetCameraYaw;
        float cameraTurnSpeed;
        bool cameraBusy;

        int currentStep;
        char steps[500] = {};
        float moveSpeed = 6.0f;

        Dictionary myDictionary;
        string textbox;

        int typeMode;
        int titleMusic;
        int gameMusic;
        int pauseMusic;

        Global() {
            currentStep = 0;
            cameraTurnSpeed = 150.0f;
            targetCameraYaw = ' ';
            yaw = 0.0f;
            pitch = 0.0f;
            typeMode = 0;
            vecMake(0.0f,1.0f,0.0f,cameraUp);
            vecMake(0.0f,0.0f,0.0f,cameraPos);
            vecMake(0.0f,0.0f,-1.0f,cameraFront);
            srand(time(NULL));
            pause = 0;
            xres = 640;
            yres = 480;

            mazeHeight = 30;
            mazeWidth = 30;

            if (!sound.init()) {
                fprintf(stderr, "Failed to initialize sound: %s\n", sound.lastError());
            }
            //load all music we will be using
            titleMusic = sound.loadWav("737engine.wav", true, 1.0f, 1.0f);
            gameMusic = titleMusic;
            pauseMusic = sound.loadWav("pauseMusic.wav", true, 1.0f, 1.0f);
            
            GLfloat la[]  = {  0.0f, 0.0f, 0.0f, 1.0f };
            GLfloat ld[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
            GLfloat ls[] = {  0.5f, 0.5f, 0.5f, 1.0f };
            GLfloat lp[] = { 100.0f, 60.0f, -140.0f, 1.0f };
            
            lp[0] = rnd() * 200.0 - 100.0;
            lp[1] = rnd() * 100.0 + 20.0;
            lp[2] = rnd() * 300.0 - 150.0;
            
            memcpy(lightAmbient, la, sizeof(GLfloat)*4);
            memcpy(lightDiffuse, ld, sizeof(GLfloat)*4);
            memcpy(lightSpecular, ls, sizeof(GLfloat)*4);
            memcpy(lightPosition, lp, sizeof(GLfloat)*4);
            
            lesson_num=1;
            rtri = 0.0f;
            rquad = 0.0f;
            Flt gcubeRot[3]={2.0,0.0,0.0};
            Flt gcubeAng[3]={0.0,0.0,0.0};
            
            memcpy(cubeRot, gcubeRot, sizeof(Flt)*3);
            memcpy(cubeAng, gcubeAng, sizeof(Flt)*3);

            cx = 10;
            cy = 20;
            cz = 10;

            //variables used to make sure that we do not exceed the bounds of the maze <0 or  >mazeWidth;
            int tempX = 0;
            int tempY = 0;
            int rng = rand() % 4;
            
            //initialize maze steps
            for(int i = 0; i < 500;)
            {
                cout << rng;
                //north
                if(rng == 0)
                {
                    if(tempY >= mazeWidth)
                    {
                        rng = 1;
                        continue;
                    }
                    steps[i] = 'n';
                    tempY++;
                    i++;
                }
                //south
                if(rng == 1)
                {
                    if(tempY <= 0)
                    {
                        rng = 0;
                        continue;
                    }
                    steps[i] = 's';
                        tempY--;
                    i++;
                }
                //east
                if(rng == 2)
                {
                    if(tempX >= mazeHeight)
                    {
                        rng = 3;
                        continue;
                    }
                    steps[i] = 'e';
                    tempX++;
                    i++;
                }
                //west
                if(rng == 3)
                {
                    if(tempX <= 0)
                    {
                        rng = 2;
                        continue;
                    }
                    steps[i] = 'w';
                    tempX--;
                    i++;
                }
    
            
                rng = rand() % 4;
                
            } 
            for(int i = 0; i < 500;i++)
            {
            cout << steps[i] << ", ";
            }
            cout << endl;

        }
        ~Global() {
            delete[] enemyTex;
        }
} g;

//X11 functions
class X11_wrapper {
    private:
        Display *dpy;
        Window win;
        GLXContext glc;
    public:
        X11_wrapper(void) {
            GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
            //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
            XSetWindowAttributes swa;
            setup_screen_res(640, 480);
            dpy = XOpenDisplay(NULL);
            if (dpy == NULL) {
                printf("\n\tcannot connect to X server\n\n");
                exit(EXIT_FAILURE);
            }
            Window root = DefaultRootWindow(dpy);
            XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
            if (vi == NULL) {
                printf("\n\tno appropriate visual found\n\n");
                exit(EXIT_FAILURE);
            } 
            Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                StructureNotifyMask | SubstructureNotifyMask;
            win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
                    vi->depth, InputOutput, vi->visual,
                    CWColormap | CWEventMask, &swa);
            set_title();
            glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
            glXMakeCurrent(dpy, win, glc);
        }
        ~X11_wrapper(void) {
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
        }
        void set_title(void) {
            //Set the window title bar.
            XMapWindow(dpy, win);
            XStoreName(dpy, win, "3480 lab-6");
        }
        void setup_screen_res(const int w, const int h) {
            g.xres = w;
            g.yres = h;
        }
        void reshape_window(int width, int height) {
            //window has been resized.
            setup_screen_res(width, height);
            //
            glViewport(0, 0, (GLint)width, (GLint)height);
            glMatrixMode(GL_PROJECTION); glLoadIdentity();
            glMatrixMode(GL_MODELVIEW); glLoadIdentity();
            glOrtho(0, g.xres, 0, g.yres, -1, 1);
            set_title();
        }
        void check_resize(XEvent *e) {
            //The ConfigureNotify is sent by the
            //se5ver if the window is resized.
            if (e->type != ConfigureNotify)
                return;
            XConfigureEvent xce = e->xconfigure;
            if (xce.width != g.xres || xce.height != g.yres) {
                //Window size did change.
                reshape_window(xce.width, xce.height);
            }
        }
        bool getXPending() {
            return XPending(dpy);
        }
        XEvent getXNextEvent() {
            XEvent e;
            XNextEvent(dpy, &e);
            return e;
        }
        void swapBuffers() {
            glXSwapBuffers(dpy, win);
        }
} x11;

void showPauseScreen() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable 3D stuff temporarily.
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    // Enable transparency.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw faded black rectangle over whole screen.
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);

    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(g.xres, 0);
        glVertex2f(g.xres, g.yres);
        glVertex2f(0, g.yres);
    glEnd();

    // Draw pause text.
    glEnable(GL_TEXTURE_2D);

    Rect r;
    r.bot = g.yres / 2;
    r.left = g.xres / 2 - 45;
    r.center = 0;

    ggprint16(&r, 16, 0x00ffffff, "PAUSED");
    r.bot = g.yres / 2 - 30;
    r.left = g.xres / 2 - 80;
    ggprint8b(&r, 16, 0x00ffffff, "Press Shift+P to unpause");

    // Restore OpenGL state.
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void init_opengl(void);
void DrawGLSkybox();
void init_textures(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics(void);
void render(void);
void updateCameraFront();
void updateLesson1(float dt);
void updateLesson2(float dt);
void updateScriptedCameraTurn(float dt);
void updateAutoNavigation(float dt);

int main(void)
{
    init_opengl();
   
    // init_maze();
    g.targetCameraYaw = g.steps[0];
    g.currentStep++;
    //printf("turning %c",g.targetCameraYaw);

    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    g.sound.play(g.titleMusic);
    clock_gettime(CLOCK_MONOTONIC, &timeStart);
    int done = 0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        // Run physics and rendering on fixed timers so monitor refresh
        // does not change game speed.
		clock_gettime(CLOCK_MONOTONIC, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);
        if (timeSpan < 0.0)
            timeSpan = 0.0;
        if (timeSpan > maxFrameTime)
            timeSpan = maxFrameTime;
        physicsCountdown += timeSpan;
        renderCountdown += timeSpan;
        statsCountdown += timeSpan;

		while(physicsCountdown >= physicsRate) {
            if (!g.pause) {
                physics();
            }
            ++physicsFrames;
            physicsCountdown -= physicsRate;
		}
        if (renderCountdown >= renderRate) {
            render();
            x11.swapBuffers();
            ++renderFrames;
            while (renderCountdown >= renderRate)
                renderCountdown -= renderRate;
        } else {
            double sleepTime = renderRate - renderCountdown;
            if (sleepTime > 0.0) {
                struct timespec sleepRequest;
                sleepRequest.tv_sec = (time_t)sleepTime;
                sleepRequest.tv_nsec =
                    (long)((sleepTime - (double)sleepRequest.tv_sec) * 1e9);
                nanosleep(&sleepRequest, NULL);
            }
        }
        if (statsCountdown >= 1.0) {
            displayedUPS = physicsFrames;
            displayedFPS = renderFrames;
            physicsFrames = 0;
            renderFrames = 0;
            while (statsCountdown >= 1.0)
                statsCountdown -= 1.0;
        }
    }
    cleanup_fonts();
    g.sound.shutdown();
    return 0;
}


void vecScale(Vec v0,float s0, Vec dest)
{

    dest[0] = v0[0]*s0;
    dest[1] = v0[1]*s0;
    dest[2] = v0[2]*s0;
}

#define VecCross(a,b,c) \
    (c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1]; \
    (c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2]; \
    (c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]

void vecCrossProduct(Vec v0, Vec v1, Vec dest)
{
    dest[0] = v0[1]*v1[2] - v1[1]*v0[2];
    dest[1] = v0[2]*v1[0] - v1[2]*v0[0];
    dest[2] = v0[0]*v1[1] - v1[0]*v0[1];
}

Flt vecDotProduct(Vec v0, Vec v1)
{
    return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

void vecZero(Vec v)
{
    v[0] = v[1] = v[2] = 0.0;
}

void vecMake(Flt a, Flt b, Flt c, Vec v)
{
    v[0] = a;
    v[1] = b;
    v[2] = c;
}

void vecCopy(Vec source, Vec dest)
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

Flt vecLength(Vec v)
{
    return sqrt(vecDotProduct(v, v));
}

void vecNormalize(Vec v)
{
    Flt len = vecLength(v);
    if (len == 0.0) {
        vecMake(0,0,1,v);
        return;
    }
    len = 1.0 / len;
    v[0] *= len;
    v[1] *= len;
    v[2] *= len;
}

void vecSub(Vec v0, Vec v1, Vec dest)
{
    dest[0] = v0[0] - v1[0];
    dest[1] = v0[1] - v1[1];
    dest[2] = v0[2] - v1[2];
}
void vecAdd(Vec v0, Vec v1, Vec dest)
{
    dest[0] = v0[0] + v1[0];
    dest[1] = v0[1] + v1[1];
    dest[2] = v0[2] + v1[2];
}


void init_opengl(void)
{
    //OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //gluLookAt(g.cameraPos,  g.cameraPos + g.cameraFront,  g.cameraUp);
    gluLookAt(0,5,10,  0,50,0,  0,1,1);
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable( GL_LIGHTING );
    glLightfv(GL_LIGHT0, GL_AMBIENT, g.lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, g.lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, g.lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glEnable(GL_LIGHT0);

    g.tex.backImage = &img[0];
    //create opengl texture elements
    glGenTextures(1, &g.tex.backTexture);
    int w = g.tex.backImage->width;
    int h = g.tex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.tex.backImage->data);
    g.tex.xc[0] = 0.0;
    g.tex.xc[1] = 1.0;
    g.tex.yc[0] = 0.0;
    g.tex.yc[1] = 1.0;


    g.carpetTex.backImage = &img[1];
    //create opengl texture elements
    glGenTextures(1, &g.carpetTex.backTexture);
    w = g.carpetTex.backImage->width;
    h = g.carpetTex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.carpetTex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.carpetTex.backImage->data);
    g.carpetTex.xc[0] = 0.0;
    g.carpetTex.xc[1] = 1.0;
    g.carpetTex.yc[0] = 0.0;
    g.carpetTex.yc[1] = 1.0;


    g.skyTex.backImage = &img[2];
    //create opengl texture elements

    glGenTextures(1, &g.skyTex.backTexture);
    w = g.skyTex.backImage->width;
    h = g.skyTex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.skyTex.backImage->data);
    g.skyTex.xc[0] = 0.0;
    g.skyTex.xc[1] = 1.0;
    g.skyTex.yc[0] = 0.0;
    g.skyTex.yc[1] = 1.0;


    g.asciiTex.backImage = &img[3];
    //create opengl texture elements
    glGenTextures(1, &g.asciiTex.backTexture);
    w = g.asciiTex.backImage->width;
    h = g.asciiTex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.asciiTex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.asciiTex.backImage->data);
    g.carpetTex.xc[0] = 0.0;
    g.carpetTex.xc[1] = 1.0;
    g.carpetTex.yc[0] = 0.0;
    g.carpetTex.yc[1] = 1.0;






    g.ceilingTex.backImage = &img[5];
    //create opengl texture elements
    glGenTextures(1, &g.ceilingTex.backTexture);
    w = g.ceilingTex.backImage->width;
    h = g.ceilingTex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.ceilingTex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.ceilingTex.backImage->data);
    g.ceilingTex.xc[0] = 0.0;
    g.ceilingTex.xc[1] = 1.0;
    g.ceilingTex.yc[0] = 0.0;
    g.ceilingTex.yc[1] = 1.0;

for (size_t num = 0; num < enemies.size(); num++) {
    g.enemyTex[num].backImage = &enemies[num];
    //create opengl texture elements
    w = g.enemyTex[num].backImage->width;
    h = g.enemyTex[num].backImage->height;


    glEnable(GL_TEXTURE_2D);
    unsigned char *data1 = new unsigned char [g.enemyTex[num].backImage->width * g.enemyTex[num].backImage->height * 4];
            for (int i=0; i<g.enemyTex[num].backImage->height; i++) {
                    for (int j=0; j<g.enemyTex[num].backImage->width; j++) {
                            int offset  = i*g.enemyTex[num].backImage->width*3 + j*3;
                            int offset2 = i*g.enemyTex[num].backImage->width*4 + j*4;
                            data1[offset2+0] = g.enemyTex[num].backImage->data[offset+0];
                            data1[offset2+1] = g.enemyTex[num].backImage->data[offset+1];
                            data1[offset2+2] = g.enemyTex[num].backImage->data[offset+2];
                            data1[offset2+3] =
                                    ((unsigned char)g.enemyTex[num].backImage->data[offset+0] != 0 &&
                                    (unsigned char)g.enemyTex[num].backImage->data[offset+1] != 0 &&
                                    (unsigned char)g.enemyTex[num].backImage->data[offset+2] != 0);
                    }
            }
    glGenTextures(1, &g.enemyTex[num].backTexture);
    glBindTexture(GL_TEXTURE_2D, g.enemyTex[num].backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data1);
    delete [] data1;
}


    g.weaponTex.backImage = &img[6];
    //create opengl texture elements
    
    w = g.weaponTex.backImage->width;
    h = g.weaponTex.backImage->height;


    glEnable(GL_TEXTURE_2D);
    unsigned char *data2 = new unsigned char [g.weaponTex.backImage->width * g.weaponTex.backImage->height * 4];
            for (int i=0; i<g.weaponTex.backImage->height; i++) {
                    for (int j=0; j<g.weaponTex.backImage->width; j++) {
                            int offset  = i*g.weaponTex.backImage->width*3 + j*3;
                            int offset2 = i*g.weaponTex.backImage->width*4 + j*4;
                            data2[offset2+0] = g.weaponTex.backImage->data[offset+0];
                            data2[offset2+1] = g.weaponTex.backImage->data[offset+1];
                            data2[offset2+2] = g.weaponTex.backImage->data[offset+2];
                            data2[offset2+3] =
                                    ((unsigned char)g.weaponTex.backImage->data[offset+0] != 0 &&
                                    (unsigned char)g.weaponTex.backImage->data[offset+1] != 0 &&
                                    (unsigned char)g.weaponTex.backImage->data[offset+2] != 0);
                    }
            }
    glGenTextures(1, &g.weaponTex.backTexture);
    glBindTexture(GL_TEXTURE_2D, g.weaponTex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data2);
    delete [] data2;
    // g.weaponTex.xc[0] = 0.0;
    // g.weaponTex.xc[1] = 1.0;
    // g.weaponTex.yc[0] = 0.0;
    // g.weaponTex.yc[1] = 1.0;



}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button is down
        }
        if (e->xbutton.button==3) {
            //Right button is down
        }
    }
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        //Mouse moved
        savex = e->xbutton.x;
        savey = e->xbutton.y;
    }
}

int check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);

    if (e->type == KeyPress) {
       g.keys[key] = true;
    }

    if (e->type == KeyRelease) {
        g.keys[key] = false;
        return 0;
    }

    if (g.keys[XK_Shift_L] && key == XK_p) {
        g.pause = !g.pause;
        if (g.pause) {
            g.sound.stop(g.titleMusic);
            g.sound.play(g.pauseMusic);
        } else {
            g.sound.stop(g.pauseMusic);
            g.sound.play(g.titleMusic);
        }
        g.keys[XK_p] = false; // prevents toggling repeatedly
        return 0;
    }


    if(!g.typeMode)
    {
        updateCameraFront();
        const float keyMoveStep = g.moveSpeed * (float)physicsRate;
        switch(key) {
            case XK_1:
                g.lesson_num = 1;
                init_opengl();
                break;
            case XK_2:
                g.lesson_num = 2;
                init_opengl();
                break;
            case XK_z:
                g.targetCameraYaw = 'e';
                break;
            case XK_x:
                g.targetCameraYaw = 'w';
                break;
            case XK_3:
                g.targetCameraYaw = 'n';
                break;
            case XK_4:
                g.targetCameraYaw = 's';
                break;
            case XK_5:
                g.lesson_num = 5;
                init_opengl();
                break;
            case XK_6:
                g.lesson_num = 6;
                init_opengl();
                break;
            case XK_l:
                //set light position
                g.lightPosition[0] = rnd() * 200.0 - 100.0;
                g.lightPosition[1] = rnd() * 100.0 + 20.0;
                g.lightPosition[2] = rnd() * 300.0 - 150.0;
                glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
                break;
            case XK_Escape:
                return 1;
            case XK_m:
                g.lesson_num = 'm';
                g.cameraPos[0] = 3.5;
                g.cameraPos[2] = -5;
                init_opengl();
                break;
            case XK_Up:
                Vec cfuTemp;
                vecScale(g.cameraFront,keyMoveStep,cfuTemp);
                vecAdd(g.cameraPos,cfuTemp,g.cameraPos);
                break;
            case XK_Down:
                Vec cfdTemp;
                vecScale(g.cameraFront,keyMoveStep,cfdTemp);
                vecSub(g.cameraPos,cfdTemp,g.cameraPos);
                break;
            case XK_Left:
                Vec crossTempL;
                vecCrossProduct(g.cameraFront,g.cameraUp,crossTempL);
                vecNormalize(crossTempL);
                vecScale(crossTempL,keyMoveStep,crossTempL);
                vecSub(g.cameraPos,crossTempL,g.cameraPos);
                break;
            case XK_Right:
                Vec crossTempR;
                vecCrossProduct(g.cameraFront,g.cameraUp,crossTempR);
                vecNormalize(crossTempR);
                vecScale(crossTempR,keyMoveStep,crossTempR);
                vecAdd(g.cameraPos,crossTempR,g.cameraPos);
                break;
            case XK_o:
                //g.cy -=1;
                g.yaw-=1;
                g.targetCameraYaw = ' ';
                updateCameraFront();
                printf("%f, ",g.cameraFront[0]);
                printf("%f\n",g.cameraFront[2]);
                break;
            case XK_p:
                g.yaw+=1;
                g.targetCameraYaw = ' ';
                updateCameraFront();
                printf("%f, ",g.cameraFront[0]);
                printf("%f\n",g.cameraFront[2]);
                break;
            case XK_k:
                g.pitch +=1;
                updateCameraFront();
                break;
            case XK_j:
                g.pitch -=1;
                updateCameraFront();
                break;
            case XK_u:
                g.cameraPos[1] +=1;
                break;
            case XK_t:
                cout << "now typing..." << endl;
                g.typeMode = 1;
                break;
            case XK_i:
                g.cameraPos[1] -=1;
                break;
        }
    }
    else
    {
        if(key >= 97 && key <= 122) // a-z
            currentText.push((char)key);
        if(key == 32)  //space
            currentText.push(' ');   
        if(key == XK_Escape)  //escape
        {
            g.typeMode = 0;
            cout << "no longer typing" << endl;
        }   
        if(key == XK_BackSpace) //backspace
            if(!currentText.empty())
                currentText.pop();
        if(key == 48) //idek, i though this was backspace but i guess not
            if(!currentText.empty())
                currentText.pop();
    }
    return 0;
}



void updateCameraFront()
{
    g.cameraFront[0] = cos(g.yaw * (2.0f * (PI /180.0f))) *
        cos(g.pitch * (2.0f * (PI/180.0f)));
    g.cameraFront[1] = sin(g.pitch * (2.0f * (PI/180.0f)));
    g.cameraFront[2] = sin(g.yaw * (2.0f * (PI /180.0f))) *
        cos(g.pitch * (2.0f * (PI/180.0f)));
}

void updateScriptedCameraTurn(float dt)
{   


    int target;
    switch (g.targetCameraYaw)
    {
        case 'n':
            target = 135;
            break;
        case 's':
            target = 45;
            break;
        case 'w':
            target = 90;
            break;
        case 'e':
            target = 0;
            break;
        case ' ':
            return;
            break;

    }

    float turnStep = g.cameraTurnSpeed * dt;
    if (turnStep <= 0.0f) {
        turnStep = 0.001f;
    }

    if(g.yaw < target + turnStep && g.yaw > target - turnStep)
    {
        g.cameraBusy = false;
        g.yaw = target;

    }else
    {
        g.cameraBusy = true;
        if(target < g.yaw)
        {
            g.yaw -= turnStep;
        }
        if(target > g.yaw)
        {
            g.yaw += turnStep;
        }


    }
}
class Enemy
{
    private:
        
        Vec tpos;
        Vec pos;
        float speed;
        int numCorrect;
        int textureIndex;
        // Texure texture;
    public:
        string word;
        Enemy()
        {
            //   texture;
            //srand(time(NULL));
            word = g.myDictionary.getRandomWord();
            tpos[0] = rand() % (g.xres - 30);
            tpos[1] = rand() % (g.yres - 30);
            
            speed = 2.0f;
          
          
            pos[0] = rand() % 500;
            pos[0] = float(pos[0]) / 100;
            pos[1] = 0;
            pos[2] = rand() % 500;
            pos[2] = float(pos[2]) / 100;
            textureIndex = rand() % enemies.size();
          
          //pos[0] = 0;
          //pos[1] = 0;
          //pos[2] = 0;
            numCorrect = 0;
            cout << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
        }
        void update(float dt)
        {
            
                Vec playerDir;
                vecMake(g.cameraPos[0],g.cameraPos[1],g.cameraPos[2],playerDir);
                vecSub(g.cameraPos,pos,playerDir);
                vecNormalize(playerDir);
                vecScale(playerDir, speed * dt, playerDir);
                vecAdd(pos,playerDir,pos);
                numCorrect = 0;
                for(int i = 0; i < (int)g.textbox.length();i++)
                {
                    if(g.textbox[i] != word[i])
                        break;
                    numCorrect = i + 1;
                }
        }
        string getWord()
        {
            return word;
        }
        float getX()
        {
            return pos[0];
        }
        float getY()
        {
            return pos[1];
        }
        void drawString(string input)
        {
            int wl = input.length();
            const string textureKey = " ! #$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[ ]^_ abcdefghijklmnopqrstuvwxyz{|}~ ........................";


                
                float scale = 0.05f;
                glPushMatrix();

                
                
               
                Vec playerDir;
           
                playerDir[0] =   g.cameraPos[0] - pos[0]; 
                playerDir[2] =  g.cameraPos[2] - pos[2];

                
                //vecNormalize(playerDir);
                float adj_angle = -(atan2(playerDir[2],playerDir[0]) * (180/PI));
                int textRot = (((int)adj_angle + 360) % 360) + 90;
           
                glTranslatef(pos[0] ,0.5f,pos[2]);
                glRotatef((textRot),0.0f,1.0f,0.0f);
                glScalef(scale,scale,scale);
                glColor3f(1.0f,0.5f,0.5f);
                glBindTexture(GL_TEXTURE_2D, g.asciiTex.backTexture);
                glBegin(GL_QUADS);
            for(int i = 0; i < wl;i++)
            {

                int textureIndex = 1;
                for(int j = 0; j < (int)textureKey.length();j++)
                {
                    if(textureKey[j] == input[i])
                    {
                        textureIndex = j;
                    }

                }
                
                float ai = (2 * i) - wl;
		
		

        int ty = floor(textureIndex / 15);
		int tx = (textureIndex % 15);



		float tx1 = tx * 1.0f/15.0f;
		float tx2 = tx1 + 1.0f/15.0f;
		float ty1 = ty * 1.0f/8.0f;
		float ty2 = ty1 + 1.0f/8.0f;

		//back side
        float typeJumpAmt = 0.4f;
        if(numCorrect > 0 && i <= numCorrect - 1)
        {
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f + ai, 1.0f + typeJumpAmt, 0.0f);
		glVertex3f( 1.0f + ai, 1.0f + typeJumpAmt, 0.0f);
		glVertex3f( 1.0f + ai,-1.0f + typeJumpAmt, 0.0f);
		glVertex3f(-1.0f + ai,-1.0f + typeJumpAmt, 0.0f);

        glColor3f(0.5f,0.5f,0.5f);
        glTexCoord2f(tx1, ty2); glVertex3f(-1.0f + ai, -1.0f + typeJumpAmt, 0.01f);
		glTexCoord2f(tx1, ty1); glVertex3f( -1.0f + ai, 1.0f + typeJumpAmt, 0.01f);
		glTexCoord2f(tx2, ty1); glVertex3f( 1.0f + ai,1.0f + typeJumpAmt, 0.01f);
		glTexCoord2f(tx2, ty2); glVertex3f(1.0f + ai,-1.0f + typeJumpAmt, 0.01f);
        
        }
        else
        {
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f + ai, 1.0f , 0.0f);
		glVertex3f( 1.0f + ai, 1.0f, 0.0f);
		glVertex3f( 1.0f + ai,-1.0f, 0.0f);
		glVertex3f(-1.0f + ai,-1.0f, 0.0f);
        
        glColor3f(1.0f,1.0f,1.0f);
        glTexCoord2f(tx1, ty2); glVertex3f(-1.0f + ai, -1.0f , 0.01f);
		glTexCoord2f(tx1, ty1); glVertex3f( -1.0f + ai, 1.0f , 0.01f);
		glTexCoord2f(tx2, ty1); glVertex3f( 1.0f + ai,1.0f , 0.01f);
		glTexCoord2f(tx2, ty2); glVertex3f(1.0f + ai,-1.0f , 0.01f);
        
        }
        
        //front
		glNormal3f( 0.0f, 0.0f, 1.0f);
	    }
	    glEnd();
	    glPopMatrix();
	    glBindTexture(GL_TEXTURE_2D, 0);

	}
	void drawEnemy()
	{

		float scale = 0.4f;
		glPushMatrix();
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER,0.0f);
            
           
            
            Vec playerDir;
            playerDir[0] =   g.cameraPos[0] - pos[0]; 
            playerDir[2] =  g.cameraPos[2] - pos[2];
            float adj_angle = -(atan2(playerDir[2],playerDir[0]) * (180/PI));
            int enemyRot = (((int)adj_angle + 360) % 360) + 90;

            glColor4ub(255,255,255,255);
            glTranslatef(pos[0] ,(1 * scale)-1,pos[2]);
            glRotatef((enemyRot),0.0f,1.0f,0.0f);
            glScalef(scale,scale,scale);
            glColor3f(1.0f,1.0f,1.0f);
            glBindTexture(GL_TEXTURE_2D, g.enemyTex[textureIndex].backTexture);
            
            glBegin(GL_QUADS);
                glNormal3f( 0.0f, 0.0f, 1.0f);
                glColor3f(1.0f,1.0f,1.0f);
                glColor3f(1.0f,1.0f,1.0f);
                glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.01f);
                glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.01f);
                glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.01f);
                glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.01f);
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_ALPHA_TEST);   
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void draw()
	{
		glPushMatrix();
		Rect r;
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, g.xres, 0, g.yres, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_LIGHTING);
		r.bot = tpos[1];
		r.left = tpos[0];
		r.center = 0;
		ggprint8b(&r, 16, 0x00990000, "%s",word.c_str());
		glPopMatrix();



		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(90.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);


		glDisable(GL_LIGHTING);
		drawString(word);
		drawEnemy();
		glEnable(GL_LIGHTING);


	}
	bool checkMatch(string in_string)
	{
        
		if(in_string == word)
			return true;
		else 
			return false;
	}
};



void createTile(int x, int y, int z, bool n, bool e, bool s, bool w)
{

    const int width= 1;

	//FLOOR
    glColor3f(1.0f,1.0f,1.0f);

	glPushMatrix();
        glTranslatef(x,y-1.0f,z);
        glRotatef(-90.0f,1.0f,0.0f,0.0f);
        glColor3f(1.0f,1.0f,1.0f);
        glBindTexture(GL_TEXTURE_2D, g.carpetTex.backTexture);
        glBegin(GL_QUADS);
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f); 
        glEnd();
	glPopMatrix();


    //ceiling
    glPushMatrix();
        glTranslatef(x,y+1.0f,z);
        glRotatef(90.0f,1.0f,0.0f,0.0f);
        glColor3f(1.0f,1.0f,1.0f);
        glBindTexture(GL_TEXTURE_2D, g.ceilingTex.backTexture);
        glBegin(GL_QUADS);
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f);
        glEnd();
	glPopMatrix();

	if(n)
	{
        //North wall
		glPushMatrix();
            glTranslatef(x,y+0.0f,z+width);
            glRotatef(180.0f,0.0f,1.0f,0.0f);
            glColor3f(1.0f,1.0f,1.0f);
            glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
            glBegin(GL_QUADS);
                glNormal3f( 0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f);
            glEnd();
		glPopMatrix();
	}
	if(w)
	{
    	//West WALL
		glPushMatrix();
            glTranslatef(x-width,y+0.0f,z);
            glRotatef(90.0f,0.0f,1.0f,0.0f);
            glColor3f(1.0f,1.0f,1.0f);
            glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
            glBegin(GL_QUADS);
                glNormal3f( 0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f);
            glEnd();
		glPopMatrix();
	}
	if(e)
	{
		//East WALL
		glPushMatrix();
            glTranslatef(x+width,y+0.0f,z);
            glRotatef(-90.0f,0.0f,1.0f,0.0f);
            glColor3f(1.0f,1.0f,1.0f);
            glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
            glBegin(GL_QUADS);
                glNormal3f( 0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f);  
            glEnd();
		glPopMatrix();
	}
	if(s)
	{
		glPushMatrix();
            glTranslatef(x,y+0.0f,z-width);
            glRotatef(180.0f,0.0f,0.0f,0.0f);
            glColor3f(1.0f,1.0f,1.0f);
            glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
            glBegin(GL_QUADS);
                glNormal3f( 0.0f, 0.0f, 1.0f);
                glColor3f(1.0f,1.0f,1.0f);
                glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f);glVertex3f( 1.0f, 1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f);glVertex3f( 1.0f,-1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f,-1.0f, 0.0f);
            glEnd();
		glPopMatrix();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawMap()
{
    int height = g.mazeHeight;
    int width = g.mazeWidth;

    for (int i = -height; i < height; i++)
    {
        for (int j = -width; j < width; j++)
        {
            if (j % 2 != 0 && i % 2 == 0)
                createTile(i * 2, 0, j * 2, false, true, false, true);
            else if (j % 2 == 0 && i % 2 != 0)
                createTile(i * 2, 0, j * 2, true, false, true, false);
            else
                createTile(i * 2, 0, j * 2, false, false, false, false);
        }
    }
}

const int nEnemies = 4;
Enemy * debugEnemy[nEnemies];

void TypeDebug()
{

	static int firstRun = 1;
	if(firstRun)
	{
		for(int i = 0; i < nEnemies;i++)
		{
			debugEnemy[i] = new Enemy();
		}
        g.targetCameraYaw = 'n';
	}
	firstRun = 0;
	static string rWord = g.myDictionary.getRandomWord();
	rWord = g.myDictionary.getRandomWord();
	//g.targetCameraYaw = ' ';
	
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawGLSkybox();

    	g.textbox = "";
	Rect r;

	


	int count = 0;    
	stack<char> printStack = currentText;
	while(!printStack.empty())
	{
		g.textbox.push_back(printStack.top());
		printStack.pop();
		count++;
	}
	
	std::reverse(g.textbox.begin(),g.textbox.end());
	std::transform(g.textbox.begin(), g.textbox.end(), g.textbox.begin(),
		    [](unsigned char c){ return static_cast<unsigned char>(std::toupper(c)); });



	for(int i =0; i < nEnemies;i++)
	{

	    if(debugEnemy[i]->checkMatch(g.textbox))
	    {
		debugEnemy[i] = new Enemy();
		stack<char> emptyText;
		currentText =  emptyText;
	    }
	    //cout << g.textbox << endl;
	  //  cout << debugEnemy[i]->word << endl;
	    debugEnemy[i]->draw();
	}

	drawMap();



	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	r.bot = 0;
	r.left = 0;
	r.center = 0;
	ggprint8b(&r, 16, 0x00990000, "%s",g.textbox.c_str());

    //glPushMatrix();
    glColor3ub(255,255,255);
    //glTranslatef(x+1.5f,y+1.0f,z-5.0f);
	//glRotatef(90.0f,1.0f,0.0f,0.0f);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glColor4ub(255,255,255,255);
    glBindTexture(GL_TEXTURE_2D, g.weaponTex.backTexture);
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, 0.0f, 1.0f);
	
    float w = g.xres /2;
    float h = g.yres /2;

	glTexCoord2f(0.0f, 0.0f);glVertex2f(0, h);
	glTexCoord2f(1.0f, 0.0f);glVertex2f( w, h);
	glTexCoord2f(1.0f, 1.0f);glVertex2f( w,0);
	glTexCoord2f(0.0f, 1.0f);glVertex2f(0,0);
	glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);   
    //glDisable(GL_BLEND);
	//glPopMatrix();
    glPopMatrix();
    
}
void DrawGame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawGLSkybox();
    //glLoadIdentity();
    glPushMatrix();
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glTranslatef(-1.5f,-0.0f,-6.0f);
    glRotatef(50,0.0f,1.0f,0.0f);
    glColor3f(0.6f,0.7f,0.8f);
    glBegin(GL_TRIANGLES);
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
    //back side
    glNormal3f( 0.0f, 0.0f, 1.0f);
    glVertex3f( 0.0f, 1.0f, 0.01f);
    glVertex3f( 1.0f,-1.0f, 0.01f);
    glVertex3f(-1.0f,-1.0f, 0.01f);
    glEnd();
    glPopMatrix();


    glBindTexture(GL_TEXTURE_2D, 0);
    g.rtri  += 4.0f;


    createTile(0,0,0,true,true,true,true);
    /*
       if(!g.cameraBusy)
       {
       static float distance = 0;
       const float err =0.1f;
       Vec cfuTemp; //camera front updated temp
       vecScale(g.cameraFront,g.moveSpeed,cfuTemp);
       vecAdd(g.cameraPos,cfuTemp,g.cameraPos);
       char cur = g.steps[g.currentStep];
       distance+=g.moveSpeed;
       if(distance <= 2.0f+err && distance >=2.0f-err)
       {
       g.targetCameraYaw = cur;
       printf("turning %c",g.targetCameraYaw);
       g.currentStep++;
       distance=0;
       }
       fflush(stdout);
       }
       */
}


void DrawGLSkybox()
{

    float scale = 50.0f;
    glPushMatrix();

    glDisable(GL_LIGHTING);

    glTranslatef(0.0f,0.0f,-scale/1.0f);
    glRotatef(0,1.0f,0.0f,0.0f);
    glScalef(scale,scale,scale);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glBegin(GL_QUADS);
    //back side
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);

    //front
    ////bind()
    glNormal3f( 0.0f, 0.0f, 1.0f);


    glColor3f(1.0f,1.0f,1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 0.01f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 0.01f);

    //bind(0)    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glPopMatrix();


    glPushMatrix();
    glDisable(GL_LIGHTING);
    glTranslatef(0.0f,0.0f,scale/1.0f);
    glRotatef(180,0.0f,1.0f,0.0f);
    glScalef(scale,scale,scale);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glBegin(GL_QUADS);
    //back side
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);

    //front
    ////bind()
    glNormal3f( 0.0f, 0.0f, 1.0f);


    glColor3f(1.0f,1.0f,1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 0.01f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 0.01f);

    //bind(0)    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    //////
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glTranslatef(-scale/1.0f,0.0f,0.0f);
    glRotatef(90,0.0f,1.0f,0.0f);
    glScalef(scale,scale,scale);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glBegin(GL_QUADS);
    //back side
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);

    //front
    ////bind()
    glNormal3f( 0.0f, 0.0f, 1.0f);


    glColor3f(1.0f,1.0f,1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 0.01f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 0.01f);

    //bind(0)    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glPopMatrix();


    //////
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glTranslatef(scale/1.0f,0.0f,0.0f);
    glRotatef(270,0.0f,1.0f,0.0f);
    glScalef(scale,scale,scale);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glBegin(GL_QUADS);
    //back side
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);

    //front
    ////bind()
    glNormal3f( 0.0f, 0.0f, 1.0f);


    glColor3f(1.0f,1.0f,1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 0.01f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 0.01f);

    //bind(0)    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glPopMatrix();



    //////top
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glTranslatef(0.0f,scale,0.0f);
    glRotatef(90,1.0f,0.0f,0.0f);
    glScalef(scale,scale,scale);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.skyTex.backTexture);
    glBegin(GL_QUADS);
    //back side
    glNormal3f( 0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);

    //front
    ////bind()
    glNormal3f( 0.0f, 0.0f, 1.0f);


    glColor3f(1.0f,1.0f,1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.01f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 0.01f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 0.01f);

    //bind(0)    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glPopMatrix();


}


void updateLesson1(float dt)
{
    updateAutoNavigation(dt);
    for (int i = 0; i < nEnemies; i++) {
        if (debugEnemy[i]) {
            debugEnemy[i]->update(dt);
        }
    }
}

void updateAutoNavigation(float dt)
{
    static float distance = 0.0f;
    const float moveStep = g.moveSpeed * dt;
    const float err = 0.1f;

    updateScriptedCameraTurn(dt);
    updateCameraFront();

    if (!g.cameraBusy) {
        Vec cfuTemp;
        vecScale(g.cameraFront, moveStep, cfuTemp);
        vecAdd(g.cameraPos, cfuTemp, g.cameraPos);
        distance += moveStep;

        if (g.currentStep < 500 &&
                distance <= 4.0f + err && distance >= 4.0f - err) {
            g.targetCameraYaw = g.steps[g.currentStep];
            printf("turning %c", g.targetCameraYaw);
            g.currentStep++;
            distance = 0.0f;
            fflush(stdout);
        }
    }
}

void updateLesson2(float dt)
{
    updateAutoNavigation(dt);
}

void physics(void)
{
    const float dt = (float)physicsRate;
//    cout << g.lesson_num << endl;
    switch (g.lesson_num) {
        case 1:
            updateLesson1(dt);
            break;
        case 2:
            updateLesson2(dt);
            break;
        default:
            updateCameraFront();
            break;
    }
}

void render(void)
{



    glClear(GL_COLOR_BUFFER_BIT);



    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);



    //
    updateCameraFront();
    Vec added;
    vecAdd(g.cameraPos,g.cameraFront,added);
    gluLookAt((double)g.cameraPos[0],(double)g.cameraPos[1],(double)g.cameraPos[2],  (double)added[0],(double)added[1],(double)added[2],(double)g.cameraUp[0],(double)g.cameraUp[1],(double)g.cameraUp[2]);
    switch (g.lesson_num) {
        case 0:break;              
        case 1: TypeDebug(); break;
        case 2: DrawGame(); break;

    }
    //Set 2D mode (no perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Rect r;
    glDisable(GL_LIGHTING);
    r.bot = g.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00887766, "3480");
    //ggprint8b(&r, 16, 0x008877aa, "L - change light position");
    ggprint8b(&r, 16, 0x008877aa, "1 - type debug");
    ggprint8b(&r, 16, 0x008877aa, "2 -  game");
    ggprint8b(&r, 16, 0x008877aa, "FORWARD,LEFT,BACK,RIGHT - MOVE");
    ggprint8b(&r, 16, 0x008877aa, "O,P - TURN");
    ggprint8b(&r, 16, 0x008877aa, "U,I - UP,DOWN");
    ggprint8b(&r, 16, 0x008877aa, "J,K - TILT UP,TILT DOWN");
    ggprint8b(&r, 16, 0x008877aa, "SHIFT+P - PAUSE");
    ggprint8b(&r, 16, 0x00ffff00, "FPS: %d", displayedFPS);
    if (g.pause) {
        showPauseScreen();
    }
}
