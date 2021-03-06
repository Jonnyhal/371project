//cs371 Fall 2013
//Jonathan Halbrook
//AJ
//program: planet.c
//
//author:  Gordon Griesel
//date:    2013
//
//This program demonstrates a sphere with texture
//
//Depending on your Linux distribution,
//may have to install these packages:
// libx11-dev
// libglew1.6
// libglew1.6-dev
//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
//#include "log.h"
#include "fonts.h"
#include "ppm.h"

typedef float Flt;
typedef Flt Vec[3];
#define rnd() (float)rand() / (float)RAND_MAX
#define PI 3.14159265358979323846264338327950
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

void initXWindows(void);
void init_opengl(void);
void init_textures(void);
void cleanupXWindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void check_keys(XEvent *e);
void physics(void);
void render(void);

int done=0;
int xres=640, yres=480;

int lesson_num=0;

float planetPos[3]={0.0,0.0,0.0};
float planetRot[3]={0.5,0.0,0.0};
float planetAng[3]={0.0,0.0,0.0};
float moonPos[3]={6.0,0.0,0.0};
float moonRot[3]={0.0,1.0,1.5};
float moonAng[3]={1.0,1.0,0.0};
float satellitePos[3]={1.0,0.0,0.0};
float satelliteRot[3]={0.0,0.0,3.0};
float satelliteAng[3]={0.0,0.0,0.0};
float starsPos[3]={0.0,0.0,0.0};
float starsRot[3]={0.5,0.0,0.0};
float starsAng[3]={0.0,0.0,0.0};
int cameraOnEarth=0;
int changelight = 0;


GLfloat LightAmbient[]  = { 0.4f, 0.4f, 0.4f, 5.0f };
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 5.0f };
GLfloat LightPosition[] = { 60.0f, 60.0f, 80.0f, 0.0f };


Ppmimage *bballImage=NULL;
GLuint bballTextureId;
Ppmimage *landImage=NULL;
GLuint landTextureId;
Ppmimage *waterImage=NULL;
GLuint waterTextureId;
Ppmimage *cloudImage=NULL;
GLuint cloudTextureId;
Ppmimage *moonImage=NULL;
GLuint moonTextureId;
Ppmimage *starImage=NULL;
GLuint starTextureId;

int main(void)
{
    initXWindows();
    init_opengl();
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    init_textures();
    while(!done) {
	while(XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_resize(&e);
	    check_mouse(&e);
	    check_keys(&e);
	}
	physics();
	render();
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    cleanup_fonts();
    return 0;
}

void cleanupXWindows(void)
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "CS371 Earth");
}

void setup_screen_res(const int w, const int h)
{
    xres = w;
    yres = h;
}

void initXWindows(void)
{
    Window root;
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;

    setup_screen_res(999, 666);
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL) {
	printf("\n\tcannot connect to X server\n\n");
	exit(EXIT_FAILURE);
    }
    root = DefaultRootWindow(dpy);
    vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
	printf("\n\tno appropriate visual found\n\n");
	exit(EXIT_FAILURE);
    } 
    //else {
    //	// %p creates hexadecimal output like in glxinfo
    //	printf("\n\tvisual %p selected\n", (void *)vi->visualid);
    //}
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
	    vi->depth, InputOutput, vi->visual,
	    CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
    //window has been resized.
    setup_screen_res(width, height);
    //
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, xres, 0, yres, -1, 1);
    set_title();
}

void init_textures(void)
{
    //land

    landImage = ppm6GetImage("./land.ppm");
    //landImage = ppm6GetImage("./land1.ppm");
    glGenTextures(1, &landTextureId);
    int w = landImage->width;
    int h = landImage->height;
    //
    glBindTexture(GL_TEXTURE_2D, landTextureId);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, landImage->data);

    //water
    waterImage = ppm6GetImage("./water.ppm");
    glGenTextures(1, &waterTextureId);
    int w1 = waterImage->width;
    int h1 = waterImage->height;
    //init_textures(waterImage, waterTextureId);
    //glGenTextures(1, &silhouette_water);
    //
    glBindTexture(GL_TEXTURE_2D, waterTextureId);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    //unsigned char *silhouetteWaterData = buildAlphaData(waterImage);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, waterImage->data);


    //clouds
    cloudImage = ppm6GetImage("./cloud1.ppm");
    glGenTextures(1, &cloudTextureId);
    int w2 = cloudImage->width;
    int h2 = cloudImage->height;
    //
    glBindTexture(GL_TEXTURE_2D, cloudTextureId);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w2, h2, 0, GL_RGB, GL_UNSIGNED_BYTE, cloudImage->data);

    //moon
    moonImage = ppm6GetImage("./moon1.ppm");
    glGenTextures(1, &moonTextureId);
    int w3 = moonImage->width;
    int h3 = moonImage->height;
    //
    glBindTexture(GL_TEXTURE_2D, moonTextureId);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w3, h3, 0, GL_RGB, GL_UNSIGNED_BYTE, moonImage->data);

    //stars
    starImage = ppm6GetImage("./stars2.ppm");
    glGenTextures(1, &starTextureId);
    int w4 = starImage->width;
    int h4 = starImage->height;
    //
    glBindTexture(GL_TEXTURE_2D, starTextureId);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w4, h4, 0, GL_RGB, GL_UNSIGNED_BYTE, starImage->data);


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


void init_opengl(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(9.0f,(GLfloat)xres/(GLfloat)yres,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable( GL_LIGHTING );
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
    glEnable(GL_LIGHT0);
}

void check_resize(XEvent *e)
{
    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify)
	return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != xres || xce.height != yres) {
	//Window size did change.
	reshape_window(xce.width, xce.height);
    }
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

void check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	switch(key) {
	    case XK_1:
		if (++cameraOnEarth > 4)
		    cameraOnEarth = 0;
		break;
	    case XK_r:
		LightPosition[0] = 100;
		LightPosition[1] = 100;
		LightPosition[2] = 100;
		moonPos[0] = 6.0;
		moonPos[1] = 0.0;
		moonPos[2] = 0.0;
		break;	
	    case XK_a:
		LightPosition[0] *= -1.0;
		break;
	    case XK_s:
		LightPosition[1] *= -1.0;
		break;
	    case XK_d:
		LightPosition[2] *= -1.0;
		break;
	    case XK_Escape:
		done=1;
		break;
	}
    }
}

void physics(void)
{
    starsAng[1] += .01;
    planetAng[1] += .25;
}

void drawMoon(float verts[19][32][3], float norms[19][32][3], float tx[19][34][2]) {
    int i, j, i2, j2, j3;
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, moonTextureId);
    glBegin(GL_QUADS);
    for (i=0; i<18; i++) {
	for (j=0; j<32; j++) {
	    i2 = i+1;
	    j2 = j+1;
	    if (j2 >= 32) j2 -= 32;
	    j3 = j+1;
	    glNormal3fv(norms[i ][j ]);
	    glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
	    glNormal3fv(norms[i2][j ]);
	    glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
	    glNormal3fv(norms[i2][j2]);
	    glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
	    glNormal3fv(norms[i ][j2]);
	    glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
	}
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void DrawPlanet()
{
    static int firsttime=1;
    //16 longitude lines.
    //8 latitude levels.
    //3 values each: x,y,z.
    int i, j, i2, j2, j3;
    static float verts[19][32][3];
    static float norms[19][32][3];
    static float    tx[19][34][2];
    if (firsttime) {
	//build basketball vertices here. only once!
	firsttime=0;
	float circle[32][2];
	float angle=0.0, inc = (PI * 2.0) / 32.0;
	for (i=0; i<32; i++) {
	    circle[i][0] = cos(angle);
	    circle[i][1] = sin(angle);
	    angle += inc;
	}
	//use the circle points to build all vertices.
	//8 levels of latitude...
	for (i=0; i<=18; i++) {
	    for (j=0; j<32; j++) {
		verts[i][j][0] = circle[j][0] * circle[i][1]; 
		verts[i][j][2] = circle[j][1] * circle[i][1];
		verts[i][j][1] = circle[i][0];
		norms[i][j][0] = verts[i][j][0]; 
		norms[i][j][1] = verts[i][j][1];
		norms[i][j][2] = verts[i][j][2];
		tx[i][j][0] = (float)j / 32.0;
		tx[i][j][1] = (float)i / 18.0;
	    }
	    tx[i][j][0] = (float)j / 32.0;
	    tx[i][j][1] = (float)i / 18.0;
	}
    }
    //
    //Land
    glPushMatrix();
    glTranslatef(planetPos[0],planetPos[1],planetPos[2]);
    glRotatef(planetAng[1],0.0f,1.0f,0.0f);
    //
    //draw the planet, made out of quads...
    glScalef(2.0f,2.0f,2.0f);
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, landTextureId);
    glBegin(GL_QUADS);
    for (i=0; i<18; i++) {
	for (j=0; j<32; j++) {
	    i2 = i+1;
	    j2 = j+1;
	    if (j2 >= 32) j2 -= 32;
	    j3 = j+1;
	    glNormal3fv(norms[i ][j ]);
	    glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
	    glNormal3fv(norms[i2][j ]);
	    glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
	    glNormal3fv(norms[i2][j2]);
	    glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
	    glNormal3fv(norms[i ][j2]);
	    glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
	}
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();

    //Water
    glPushMatrix();
    glTranslatef(planetPos[0],planetPos[1],planetPos[2]);
    glRotatef(planetAng[1],0.0f,1.0f,0.0f);
    glBindTexture(GL_TEXTURE_2D, waterTextureId);
    glBlendFunc(GL_ONE, GL_ONE);
    //draw the water, made out of quads...
    glScalef(2.01f,2.01f,2.01f);
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glBegin(GL_QUADS);
    for (i=0; i<18; i++) {
	for (j=0; j<32; j++) {
	    i2 = i+1;
	    j2 = j+1;
	    if (j2 >= 32) j2 -= 32;
	    j3 = j+1;
	    glNormal3fv(norms[i ][j ]);
	    glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
	    glNormal3fv(norms[i2][j ]);
	    glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
	    glNormal3fv(norms[i2][j2]);
	    glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
	    glNormal3fv(norms[i ][j2]);
	    glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
	}
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();

    //Clouds
    glPushMatrix();
    glTranslatef(planetPos[0],planetPos[1],planetPos[2]);
    glRotatef(planetAng[1],0.0f,1.0f,0.0f);
    glBindTexture(GL_TEXTURE_2D, cloudTextureId);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //
    //draw the clouds, made out of quads...
    glScalef(2.05f,2.05f,2.05f);
    glColor4f(1.0, 1.0, 1.0, 0.30f);
    glBegin(GL_QUADS);
    for (i=0; i<18; i++) {
	for (j=0; j<32; j++) {
	    i2 = i+1;
	    j2 = j+1;
	    if (j2 >= 32) j2 -= 32;
	    j3 = j+1;
	    glNormal3fv(norms[i ][j ]);
	    glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
	    glNormal3fv(norms[i2][j ]);
	    glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
	    glNormal3fv(norms[i2][j2]);
	    glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
	    glNormal3fv(norms[i ][j2]);
	    glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
	}
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
    //
    //draw moon
    glPushMatrix();
    glRotatef(moonAng[1],0.0f,1.0f,0.0f);
    glTranslatef(moonPos[0],moonPos[1],moonPos[2]);
    glRotatef(moonAng[1],.5f,0.5f,0.0f);
    glScalef(.5f,0.5f,0.5f);
    drawMoon(verts,norms,tx);
    glPopMatrix();
    //
    //
    moonAng[1]+= .25;
    //draw stars
    glPushMatrix();
    glTranslatef(planetPos[0],planetPos[1],planetPos[2]);
    glRotatef(starsAng[1],0.0f,0.25f,0.0f);
    glBindTexture(GL_TEXTURE_2D, starTextureId);
    //draw the stars sphere made out of quads...
    glScalef(12.01f,12.01f,12.01f);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    for (i=0; i<18; i++) {
	for (j=0; j<32; j++) {
	    i2 = i+1;
	    j2 = j+1;
	    if (j2 >= 32) j2 -= 32;
	    j3 = j+1;
	    glNormal3fv(norms[i ][j ]);
	    glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
	    glNormal3fv(norms[i2][j ]);
	    glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
	    glNormal3fv(norms[i2][j2]);
	    glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
	    glNormal3fv(norms[i ][j2]);
	    glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
	}
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void render(void)
{
    Rect r;
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //3D mode
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)xres/(GLfloat)yres, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    //This sets the camera position.
    if (cameraOnEarth) {

	//Use the rotation angle of the moon to derive its position...
	double radians = (moonAng[1] / 180.0 * 3.14159);
	radians = (3.14159 * 2.0) - radians;
	if (cameraOnEarth == 1) {
	    gluLookAt(5,0,5,  0,0,0,   0,0,10);


	} else if (cameraOnEarth == 2) {

	    Vec mPos = {(cos(radians) * 2.0f),
		moonPos[1],
		(sin(radians) * 2.0f)};
	    Vec ePos = {planetPos[0], planetPos[1], planetPos[2]};

	    //get vector from earth to moon...
	    Vec v;
	    vecSub(ePos, mPos, v);
	    vecNormalize(v);
	    mPos[0] -= v[0] * 5.0f;
	    mPos[1] -= v[1] * 5.0f;
	    mPos[2] -= v[2] * 5.0f;
	   
	    gluLookAt(
		//eye
		mPos[0], mPos[1]+1, mPos[2],
		//look at earth
		ePos[0],ePos[1],ePos[2],
		//upX, upY, upZ
		0,1,0);

	} else if (cameraOnEarth == 3){
	    Vec mPos = {cos(radians) * 2.0f, moonPos[1], sin(radians) * 2.0f};
	    gluLookAt(0,0,mPos[2]+9,  mPos[0],0,mPos[2],  0,10,0);
	} else {
	    gluLookAt(1.43,0.05,1.43,   3,0,0,    0,5,0);

	}
    } else {
	//regular camera position.
	gluLookAt(0,0,10,   0,0,0,   0,10,0);
    }

    glEnable( GL_LIGHTING );
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    //glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
    //
    DrawPlanet();
    //
    glViewport(0, 0, xres, yres);
    glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
    glMatrixMode (GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, xres, 0, yres);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    r.bot = yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00887766, "CS371 earth");
    ggprint8b(&r, 20, 0x00887766, "1 - Change camera position");
    if (cameraOnEarth==0) {
	ggprint8b(&r, 16, 0x00ff0000, "Starting Camera");
    }
    if (cameraOnEarth==1) {
	ggprint8b(&r, 16, 0x00ff0000, "Rotisserie Earth");
    }
    if (cameraOnEarth==2) {
	ggprint8b(&r, 16, 0x00ff0000, "Camera behind Moon");
    }
    if (cameraOnEarth==3) {
	ggprint8b(&r, 16, 0x00ff0000, "Camera rotates with Moons x and z values");
    }
    if (cameraOnEarth==4) {
	ggprint8b(&r, 16, 0x00ff0000, "Camera on Earth's crust");
    }
    glPopAttrib();
}



