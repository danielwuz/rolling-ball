#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

#define checkImageWidth 32
#define checkImageHeight 32

static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

static GLuint texName;

void makeCheckImage(void){
	int i,j,c;

	for(i=0;i<checkImageHeight;i++){
		for(j=0;j<checkImageWidth;j++){
			c = ((i&0x8)==0)^((j&0x8)==0);

			checkImage[i][j][0] = (GLubyte)((c==1)?255:100);
			checkImage[i][j][1] = (GLubyte)((c==2)?255:70);
			checkImage[i][j][2] = (GLubyte)((c==2)?255:0);
			checkImage[i][j][3] = (GLubyte)255;
		}
	}
}

static GLfloat plane_x[] = {1,0,0,1};
static GLfloat plane_y[] = {0,1,0,1};

static GLfloat slanted_x[] = {1,0,1,0};
static GLfloat stanted_y[] = {0,1,1,0};

static GLfloat* currentCoeff_s;
static GLfloat* currentCoeff_t;
static GLenum currentPlane;
static GLint currentGenMode;

float angle = 0;

void init(void){
	glClearColor(0,0,0,0);
	glEnable(GL_DEPTH_TEST);

	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texName);
	glBindTexture(GL_TEXTURE_2D,texName);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,checkImageWidth,checkImageHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,checkImage);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);

	currentCoeff_s = plane_x;
	currentCoeff_t = plane_y;
	currentGenMode = GL_OBJECT_LINEAR;
	currentPlane = GL_OBJECT_PLANE;

}

