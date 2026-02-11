//
//
//
//
//author:  Gordon Griesel
//OpenGL
//lab-6 starting framework
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"


#include "maze.h"
#include "mazeSolver.h"

typedef float Flt;
typedef Flt Vec[3];
void vecMake(Flt a, Flt b, Flt c, Vec v);
#define rnd() (Flt)rand() / (Flt)RAND_MAX
#define PI 3.14159265358979323846264338327950

class Image {
public:
	int width, height;
	unsigned char *data;
	~Image() { delete [] data; }
	Image(const char *fname) {
		if (fname[0] == '\0')
			return;
		char name[40];
		strcpy(name, fname);
		int slen = strlen(name);
		name[slen-4] = '\0';
		char ppmname[80];
		sprintf(ppmname,"%s.ppm", name);
		char ts[100];
		sprintf(ts, "convert %s %s", fname, ppmname);
		system(ts);
		FILE *fpi = fopen(ppmname, "r");
		if (fpi) {
			char line[200];
			fgets(line, 200, fpi);
			fgets(line, 200, fpi);
			//skip comments and blank lines
			while (line[0] == '#' || strlen(line) < 2)
				fgets(line, 200, fpi);
			sscanf(line, "%i %i", &width, &height);
			fgets(line, 200, fpi);
			//get pixel data
			int n = width * height * 3;			
			data = new unsigned char[n];			
			for (int i=0; i<n; i++)
				data[i] = fgetc(fpi);
			fclose(fpi);
		} else {
			printf("ERROR opening image: %s\n", ppmname);
			exit(0);
		}
		unlink(ppmname);
	}
};
Image img[3] = {"wall.png","carpet.png","sky4.jpg"};

class Texture {
public:
	Image *backImage;
	GLuint backTexture;
	float xc[2];
	float yc[2];
};

Maze myMaze;

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
    int cx;
    int cy;
    int cz;

    Flt yaw;
    Flt pitch;
    Vec cameraUp;
    Vec cameraPos;
    Vec cameraFront;

    char targetCameraYaw;
    int cameraTurnSpeed;
    bool cameraBusy;

    int currentStep;
    char steps[500] = {};
    float moveSpeed = 0.2f;

    Global() {
        currentStep = 0;
        cameraTurnSpeed = 5;
        targetCameraYaw = ' ';
        yaw = 0.0f;
        pitch = 0.0f;
        vecMake(0.0f,1.0f,0.0f,cameraUp);
        vecMake(0.0f,0.0f,-6.0f,cameraPos);
        vecMake(0.0f,0.0f,-1.0f,cameraFront);
        srand(time(NULL));
        xres = 640;
        yres = 480;
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
        lesson_num=3;
        rtri = 0.0f;
        rquad = 0.0f;
        Flt gcubeRot[3]={2.0,0.0,0.0};
        Flt gcubeAng[3]={0.0,0.0,0.0};
        memcpy(cubeRot, gcubeRot, sizeof(Flt)*3);
        memcpy(cubeAng, gcubeAng, sizeof(Flt)*3);

        cx = 10;
        cy = 20;
        cz = 10;
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

void init_opengl(void);
void init_maze();
void init_textures(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics(void);
void render(void);

int main(void)
{
    init_opengl();

    init_maze();
    g.targetCameraYaw = g.steps[0];
    g.currentStep++;
    printf("turning %c",g.targetCameraYaw);
    //g.cameraBusy = true;
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    int done = 0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        physics();
        render();
        x11.swapBuffers();
    }
    cleanup_fonts();
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



void init_maze()
{
    myMaze.createMaze();

    int w = myMaze.adj_gridw;
    int h = myMaze.adj_gridh;
    int nSteps = 0;
    mazeSolver(myMaze.mazeOutput,h,w,g.steps,nSteps);
    printf("%d",nSteps);

    //lists the steps that will be taken from the predertmined maze solver(random in this case).
    for(int i = 0; i < nSteps;i++)
    {
    printf("step %c\n", g.steps[i]);
    }
    fflush(stdout);
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
    if (e->type != KeyPress && e->type != KeyPress)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    switch(key) {
        case XK_1:
            g.targetCameraYaw = 'n';
            break;
        case XK_2:
            g.targetCameraYaw = 's';
            break;
        case XK_z:
            g.targetCameraYaw = 'e';
            break;
        case XK_x:
            g.targetCameraYaw = 'w';
            break;
        case XK_3:
            g.lesson_num = 3;
            init_opengl();
            break;
        case XK_4:
            g.lesson_num = 4;
            init_opengl();
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
            vecScale(g.cameraFront,g.moveSpeed,cfuTemp);
            vecAdd(g.cameraPos,cfuTemp,g.cameraPos);
            break;
        case XK_Down:
            Vec cfdTemp;
            vecScale(g.cameraFront,g.moveSpeed,cfdTemp);
            vecSub(g.cameraPos,cfdTemp,g.cameraPos);
            break;
        case XK_Left:
            Vec crossTempL;
            vecCrossProduct(g.cameraFront,g.cameraUp,crossTempL);
            vecNormalize(crossTempL);
            vecSub(g.cameraPos,crossTempL,g.cameraPos);
            break;
        case XK_Right:
            Vec crossTempR;
            vecCrossProduct(g.cameraFront,g.cameraUp,crossTempR);
            vecNormalize(crossTempR);
            vecAdd(g.cameraPos,crossTempR,g.cameraPos);
            break;
        case XK_o:
            //g.cy -=1;
            g.yaw-=1;
            g.targetCameraYaw = ' ';
            printf("%f, ",g.cameraFront[0]);
            printf("%f\n",g.cameraFront[2]);
            break;
        case XK_p:
            g.yaw+=1;
            g.targetCameraYaw = ' ';
            printf("%f, ",g.cameraFront[0]);
            printf("%f\n",g.cameraFront[2]);
            break;
        case XK_k:
            g.pitch +=1;
            break;
        case XK_j:
            g.pitch -=1;
            break;
        case XK_u:
            g.cameraPos[1] +=1;
            break;
        case XK_i:
            g.cameraPos[1] -=1;
           break;
    }
    return 0;
}

void checkCameraTurn()
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

    if(g.yaw < target+ g.cameraTurnSpeed && g.yaw > target - g.cameraTurnSpeed)
    {
        g.cameraBusy = false;
        g.yaw = target;

    }else
    {
        g.cameraBusy = true;
        if(target < g.yaw)
        {
            g.yaw -= g.cameraTurnSpeed;
        }
        if(target > g.yaw)
        {
            g.yaw+= g.cameraTurnSpeed;
        }


    }
}


void createTile(int x, int y, int z, bool n, bool e, bool s, bool w)
{
    //FLOOR
    glPushMatrix();
    //glLoadIdentity();
    glTranslatef(x+1.5f,y-1.0f,z-5.0f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.carpetTex.backTexture);
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
    glPopMatrix();

    //North wall
    if(n)
    {
        glPushMatrix();
        //glLoadIdentity();
        glTranslatef(x+1.5f,y+0.0f,z-4.0001f);
        glRotatef(180.0f,0.0f,1.0f,0.0f);
        glColor3f(1.0f,0.5f,0.5f);
        glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
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
        glPopMatrix();

    }
    if(w)
    {

        //West WALL
        //glLoadIdentity();
        glPushMatrix();
        glTranslatef(x+0.5001f,y+0.0f,z-5.0f);
        glRotatef(90.0f,0.0f,1.0f,0.0f);
        glColor3f(0.0f,0.0f,1.0f);
        glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
        glBegin(GL_QUADS);

        //back side
        glNormal3f( 0.0f, 0.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);

        //front
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

        glEnd();
        glPopMatrix();
    }
    if(e)
    {

        //East WALL
        glPushMatrix();
        //glLoadIdentity();
        glTranslatef(x+2.49999f,y+0.0f,z -5.0f);
        glRotatef(-90.0f,0.0f,1.0f,0.0f);
        glColor3f(1.0f,1.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
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
        glPopMatrix();
    }
    if(s)
    {

        glPushMatrix();
        //glLoadIdentity();
        glTranslatef(x+1.5f,y+0.0f,z-5.9999f);
        glRotatef(180.0f,0.0f,0.0f,0.0f);
        glColor3f(0.0f,1.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
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
        glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
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
void DrawGLScene3()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.5f,0.0f,-6.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();
    glTranslatef(3.0f,0.0f,0.0f);
    glColor3f(0.5f,0.5f,1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();
}

void DrawGLScene4()
{


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawGLSkybox();

    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glTranslatef(1.5f,0.0f,-6.0f);
    glRotatef(g.rtri,0.0f,1.0f,0.0f);
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
    glLoadIdentity();
    glTranslatef(1.5f,0.0f,-6.0f);
    glRotatef(g.rquad,1.0f,0.0f,0.0f);
    glColor3f(1.0f,0.5f,0.5f);
    glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
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
    g.rtri  += 4.0f;
    g.rquad -= 1.0f;
}

void DrawGLScene5()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    createTile( 0,0,0,true,false,true,false);


}



void DrawGLScene6()
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

    int adj_i = 0;
    int adj_j = 0;
    for(int i = 1; i < myMaze.adj_gridh;i+=2)
    {

        for(int j = 1; j < myMaze.adj_gridw;j+=2)
        {   
            bool in_n = true;
            bool in_e = true;
            bool in_s = true; 
            bool in_w = true;

            if(myMaze.mazeOutput[i+1][j] == ' ')
                in_n = false;
            if(myMaze.mazeOutput[i-1][j] == ' ')
                in_s = false;
            if(myMaze.mazeOutput[i][j-1] ==' ')
                in_w = false;
            if(myMaze.mazeOutput[i][j+1] == ' ')
                in_e = false;

            adj_i++;
            //printf("creating tile at %d,%d", adj_i,adj_j);
            createTile(adj_i * 2,0, adj_j * 2,in_n,in_e,in_s,in_w);

        }
        adj_j++;
        adj_i = 0;
    }


    glBindTexture(GL_TEXTURE_2D, 0);
    g.rtri  += 4.0f;
    
    checkCameraTurn();
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
    
}


void LightedCube()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //reset the camera for this scene.
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glTranslatef(0.0f,0.0f,-7.0f);
    glRotatef(g.cubeAng[0],1.0f,0.0f,0.0f);
    glRotatef(g.cubeAng[1],0.0f,1.0f,0.0f);
    glRotatef(g.cubeAng[2],0.0f,0.0f,1.0f);

    glColor3f(1.0f,1.0f,0.0f);
    glBegin(GL_QUADS);
    //top
    //notice the normal being set
    glNormal3f( 0.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    // bottom of cube
    glNormal3f( 0.0f,-1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    // front of cube
    glNormal3f( 0.0f, 0.0f, 1.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    // back of cube.
    glNormal3f( 0.0f, 0.0f,-1.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    // left of cube
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    // Right of cube
    glNormal3f( 1.0f, 0.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    glEnd();
    g.rquad -= 2.0f;
    int i;
    if (rnd() < 0.01) {
        for (i=0; i<3; i++) {
            g.cubeRot[i] = rnd() * 4.0 - 2.0;
        }
    }
    for (i=0; i<3; i++) {
        g.cubeAng[i] += g.cubeRot[i];
    }
}

void physics(void) { }

void render(void)
{


    g.cameraFront[0] = cos(g.yaw * (2.0f * (PI /180.0f))) * cos(g.pitch * (2.0f * (PI/180.0f)));
    g.cameraFront[1] = sin( g.pitch * (2.0f * (PI/180.0f)));
    g.cameraFront[2] = sin(g.yaw * (2.0f * (PI /180.0f))) * cos(g.pitch * (2.0f * (PI/180.0f)));



    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //gluLookAt(0,0,5,  0,0,0,  0,1,0);
    Vec added;
    vecAdd(g.cameraPos,g.cameraFront,added);
    gluLookAt((double)g.cameraPos[0],(double)g.cameraPos[1],(double)g.cameraPos[2],  (double)added[0],(double)added[1],(double)added[2],(double)g.cameraUp[0],(double)g.cameraUp[1],(double)g.cameraUp[2]);
    //gluLookAt(g.cx,g.cy,g.cz,  myMaze.gridw,0,myMaze.gridh,  0,1,0);
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    //
    switch (g.lesson_num) {
        case 0:
        case 3: DrawGLScene3(); break;
        case 4: DrawGLScene4(); break;
        case 5: DrawGLScene5(); break;
        case 6: LightedCube(); break;
        case 'm': DrawGLScene6(); break;
    }
    //Set 2D mode (no perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    r.bot = g.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00887766, "3480");
    ggprint8b(&r, 16, 0x008877aa, "L - change light position");
    ggprint8b(&r, 16, 0x008877aa, "M - create maze");
    ggprint8b(&r, 16, 0x008877aa, "FORWARD,LEFT,BACK,RIGHT - MOVE");
    ggprint8b(&r, 16, 0x008877aa, "O,P - TURN");
    ggprint8b(&r, 16, 0x008877aa, "U,I - UP,DOWN");
    ggprint8b(&r, 16, 0x008877aa, "J,K - TILT UP,TILT DOWN");
}


