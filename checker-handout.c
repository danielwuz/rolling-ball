#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

#define checkImageWidth 64
#define checkImageHeight 64

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

void init(void){
	//TODO what is the fourth parameter?
	glClearColor(0.529, 0.807, 0.92, 1.0);
	glEnable(GL_DEPTH_TEST);

	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,checkImageWidth,checkImageHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,checkImage);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,0,3.6,0,0,0,0,1,0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_DECAL);
	glBindTexture(GL_TEXTURE_2D,texName);

	glPushMatrix();
	glTranslatef(0.8,0,0);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-1,-1,0);
		glTexCoord2f(0,1);
		glVertex3f(-1,1,0);
		glTexCoord2f(1,1);
		glVertex3f(1,1,0);
		glTexCoord2f(1,0);
		glVertex3f(1,-1,0);
	glEnd();

	glPopMatrix();
	glTranslatef(-1.4,0,-0.6);
	glRotatef(-35, 0, 1, 0);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-1,-1,0);
		glTexCoord2f(0,1);
		glVertex3f(-1,1,0);
		glTexCoord2f(1,1);
		glVertex3f(1,1,0);
		glTexCoord2f(1,0);
		glVertex3f(1,-1,0);
	glEnd();

	glFlush();
	glDisable(GL_TEXTURE_2D);
}

void reshape(int w, int h)
{
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60,(GLfloat)w/(GLfloat)h,1,30);
}

int main1(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Checkerboard");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	
	return 0;
}