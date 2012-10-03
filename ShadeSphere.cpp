#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>

typedef float point3[3];

point3 v[]={{0,0,1},{0,0.942809,-0.33333},{-0.816497,-0.471405,-0.333333},{0.816497,-0.471405,-0.333333}};

static GLfloat theta[]={0,0,0};

int n;
int mode;

void triangle(point3 a, point3 b,point3 c){
	if(mode==0)
		glBegin(GL_LINE_LOOP);
	else
		glBegin(GL_POLYGON);
	glColor3f(0,1,0);

	if(mode ==1)
		glNormal3fv(a);
	if(mode ==2)
		glNormal3fv(a);

	glVertex3fv(a);
	
	if(mode ==2)
		glNormal3fv(b);
	glVertex3fv(b);

	if(mode ==2)
		glNormal3fv(c);
	glVertex3fv(c);

	glEnd();
};

void normal(point3 p){
	float d=0.0;
	for(int i=0;i<3;i++){
		d+=p[i]*p[i];
	}
	d=sqrt(d);
	if(d>0.0){
		for(int i=0;i<3;i++){
			p[i] /= d;
		}
	}
};

void divide_triangle(point3 a,point3 b,point3 c, int m){
	point3 v1,v2,v3;
	if(m>0){
		for(int j=0;j<3;j++){
			v1[j]=a[j]+b[j];
		}
		normal(v1);

		for(int j=0;j<3;j++){
			v2[j]=a[j]+c[j];
		}
		normal(v2);

		for(int j=0;j<3;j++){
			v3[j]=c[j]+b[j];
		}
		normal(v3);

		divide_triangle(a,v1,v2,m-1);
		divide_triangle(c,v2,v3,m-1);
		divide_triangle(b,v3,v1,m-1);
		divide_triangle(v1,v3,v2,m-1);

	}
	else {
		triangle(a,b,c);
	}
};

void tetrahedron(int m){
	divide_triangle(v[0],v[1],v[2],m);
	divide_triangle(v[3],v[2],v[1],m);
	divide_triangle(v[0],v[3],v[1],m);
	divide_triangle(v[0],v[2],v[3],m);
};

void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	mode=0;
	tetrahedron(n);
	mode=1;
	glTranslatef(-2,0,0);
	tetrahedron(n);
	mode=2;
	glTranslatef(4,0,0);
	tetrahedron(n);

	glFlush();
};

void myReshape(int w,int h){
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(w<=h){
		glOrtho(-4,4,-4*(GLfloat)h/(GLfloat)w,4*(GLfloat)h/(GLfloat)w,-10,10);
	}else{
		glOrtho(-4*(GLfloat)w/(GLfloat)h,4*(GLfloat)w/(GLfloat)h,-4,4,-10,10);
	}
	glMatrixMode(GL_MODELVIEW);
	display();

};

void myinit(){
	GLfloat light_ambient[]={0,0,0,1};
	GLfloat light_diffuse[]={1,1,1,1};
	GLfloat light_specular[]={1,1,1,1};

	glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

	GLfloat mat_specular[]={1,1,1,1};
	GLfloat mat_diffuse[]={0,1,0,0};
	GLfloat mat_ambient[]={1,1,1,1};
	GLfloat mat_shiness = 100.0;

	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialf(GL_FRONT,GL_SHININESS,mat_shiness);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1,1,1,1);
	glColor3f(0,1,0);
};

int main(int argc, char** argv){
	n = 4;
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(500,500);
	glutCreateWindow("sphere");
	myinit();
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
};