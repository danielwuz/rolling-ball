/**
CS6533 Assignment 2 : Rolling Sphere

author:		Zhe Wu
since:		03/21/2011
version:	1.0

- Implementation of  rotating sphere for question a¡¢b¡¢c¡¢d.

- Add menu entries for question e.

- Put the sample file in C:. It should look like "C:\\sphere.8"

- Run the executable file, input sphere data file path, or press ENTER to use default data file
	    
You can press 'x','y','z' to increase view point along x, y, z-axis respectively
		press 'b' to start rolling
		click right button to enable/disable rolling. 
		click left button to get more interactive options

*/
#define WINDOWS     1 /* Set to 1 for Windows, 0 else */
#define UNIX_LINUX  0 /* Set to 1 for Unix/Linux, 0 else */

#if WINDOWS
#include <windows.h>
#include <GL/glut.h>
#endif
#if UNIX_LINUX
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

#define PI 3.1415926535897

#define sqrt3f(x,y,z) sqrt(x*x+y*y+z*z)

#define DEFAULT_SPHERE_PATH "C:\\sphere.128"

struct point {
	GLfloat x, y, z;
};

/* Points data to build the sphere*/
GLfloat** sphereData;

/* The radius of the sphere*/
GLfloat radius = 1.0;

/* VRP */
point viewer = {7,3,-10};

/* Three points on rolling track: A, B, C*/
point track[] = {{-4,1,4}, {3,1,-4},{-3,1,-3}};

/* Points to draw the green ground */
point ground[] = {{5,0,8},{5,0,-4},{-5,0,-4},{-5,0,8}};

/* Index the current rolling direction*/
int currentSegment = 0, totalSegments = 3;;

/* Current position of the sphere center*/
point centerPos = track[currentSegment];

/* Vectors of the ball rolling direction*/
point* vectors;

/* Rolling axes around which the ball rotates*/
point* rotationAxis;

int data_count = -1,num,polygon_n;

/* Theta: the accumulated rotating angle */
/* Delta: the angle rotating each time */
GLfloat theta = 0.0, delta = 0.2;

/* Accumulated Matrix */
GLfloat acc_matrix[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

void display(void);
void my_init(void);
void reshape(int w, int h);
void idle(void);
void key(unsigned char key, int x, int y);
void mouse(int btn,int state, int x,int y);

/* Compute distance between two points */
float distanceAt(point p1, point p2);

/* Reset viewer to (7,3,-10) */
void setDefaultView();

bool begin = false, rolling = false;

/* file_in(): file input function. Modify here. */
void fileReadIn(void)
{
	const int MAX_FILE_LEN = 1000;
	char* filePath = new char[MAX_FILE_LEN];
	cout << "Please enter sphere data file path, or press ENTER to use the sample file("
			<< DEFAULT_SPHERE_PATH << ") " << endl;
	cin.sync();
	cin.getline(filePath,MAX_FILE_LEN);
	if(strlen(filePath) == 0){
		cout <<"No input file '"<< filePath <<"'. Use sample data."<<endl;
		filePath = DEFAULT_SPHERE_PATH;
	}

	ifstream file(filePath);
	if(file.fail()){
		cout <<"Cannot open file "<<filePath<<endl;
		exit(0);
	}
	
	//total number of triangles
	file >> num;
	cout <<"Total number of triangles: "<<num<<endl;
	
	sphereData= new GLfloat*[num];
	for(int i = 0; i < num ; i++)
	{
		int numOfPoints = 0;
		file >> numOfPoints;
		sphereData[i] = new GLfloat[3*numOfPoints];

		for(int j=0;j<3*numOfPoints;j++ )
		{
			file >> sphereData[i][j];
		}
	}
	file.close();
}

void setDefaultView(){
	viewer.x = 7.0;
	viewer.y = 3.0;
	viewer.z = -10.0;
}

void quit(){
	delete [] vectors;
	delete [] rotationAxis;

	for(int i=0;i<num;i++){
		delete [] sphereData[i];
	}

	delete [] sphereData;

	exit(1);
}

void main_menu(int index)
{
	switch(index)
	{
	case(0):                        
	{
		setDefaultView();
		break;
	}
	case(1):                        
		{
		quit();
		break;
		}
	}
	display();
}


void addMenu(){
	glutCreateMenu(main_menu);

	glutAddMenuEntry("Default View Point", 0);
	glutAddMenuEntry("Enable Lighting", NULL);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Wire Frame", 2);
	glutAddMenuEntry("Fog Options", NULL);
	glutAddMenuEntry("Texture Mapping", NULL);
	glutAddMenuEntry("Shadow", NULL);
	glutAddMenuEntry("Blending Shadow", NULL);
	glutAddMenuEntry("Shading", NULL);
	glutAddMenuEntry("Lighting", NULL);
	glutAddMenuEntry("Firework", NULL);

	glutAttachMenu(GLUT_LEFT_BUTTON);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);  // clear frame buffer (also called the color buffer)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(viewer.x,viewer.y,viewer.z,  0,0,0,  0,1,0);

	//draw the ground
	glColor3f(0.0, 1.0, 0.0);      // draw in green.
	glBegin(GL_POLYGON);
	glVertex3f(ground[0].x,ground[0].y,ground[0].z);
	glVertex3f(ground[1].x,ground[1].y,ground[1].z);
	glVertex3f(ground[2].x,ground[2].y,ground[2].z);
	glVertex3f(ground[3].x,ground[3].y,ground[3].z);
	glEnd();


	//glLineWidth(2.0);
	glColor3f(1.0,0.0,0.0);//draw in red
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(100,0,0);
	glEnd();

	glColor3f(1.0,0.0,1.0);//draw in magenta
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0,100,0);
	glEnd();

	glColor3f(0.0,0.0,1.0);//draw in blue
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0,0,100);
	glEnd();

	glTranslatef(centerPos.x,centerPos.y,centerPos.z);
	//add rotation here
	//Rotate the sphere around vector 8i+7k
	glMultMatrixf(acc_matrix);
	//glRotatef(theta,rotationAxis[currentSegment].x,rotationAxis[currentSegment].y,rotationAxis[currentSegment].z);

	glColor3f(1.0, 0.84, 0.0);      // draw in golden yellow.
	for (int a = 0; a < num; a++)
	{
		glBegin(GL_LINE_LOOP);
		glColor3f(1,0.84,0);
		glVertex3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
		glVertex3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
		glVertex3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
		glEnd();
	}

	glFlush();         // Render (draw) the object
	glutSwapBuffers(); // Swap buffers in double buffering.
}

point calculateDirection(point from, point to){
	point v;
	v.x = to.x - from.x;
	v.y = to.y - from.y;
	v.z = to.z - from.z;

	//convert v to unit-length
	float d = sqrt3f(v.x,v.y,v.z);
	v.x = v.x/d;
	v.y = v.y/d;
	v.z = v.z/d;

	return v;
}

point crossProduct(point u, point v){
	point n;
	n.x = u.y*v.z - u.z*v.y;
	n.y = u.z*v.x - u.x*v.z;
	n.z = u.x*v.y - u.y*v.x;
	return n;
}

float calculateRadius(){
	float y_max = -10000,y_min = 10000;
	for (int i = 0; i < num; i++)
	{
		{
			GLfloat y = sphereData[i][1];
			y_max = (y > y_max)?y:y_max;
			y_min = (y < y_min)?y:y_min;
		}
		{
			GLfloat y = sphereData[i][4];
			y_max = (y > y_max)?y:y_max;
			y_min = (y < y_min)?y:y_min;
		}
		{
			GLfloat y = sphereData[i][7];
			y_max = (y > y_max)?y:y_max;
			y_min = (y < y_min)?y:y_min;
		}
	}
	return (y_max-y_min)/2;
}

void my_init()
{
	//calculate the radius of the sphere
	radius = calculateRadius();
	//calculate the rolling directions
	totalSegments = sizeof(track)/sizeof(point);
	vectors = new point[totalSegments];
	rotationAxis = new point[totalSegments];
	for(int i=0;i<totalSegments - 1;i++){
		vectors[i] = calculateDirection(track[i], track[i+1]);
	}
	//and the last point to the first one
	vectors[totalSegments - 1] = calculateDirection(track[totalSegments - 1], track[0]);

	//calculate the rotating axis vectors
	point y_axis = {0,1,0};
	for(int i=0;i<totalSegments;i++){
		rotationAxis[i] = crossProduct(y_axis,vectors[i]);
	}

	glClearColor(0.529,0.807,0.92,0);
}

double calculateFovy(){
	//distance between camera and the center of object
	point obj_center = {0,radius,2};
	double distance = distanceAt(viewer,obj_center);
	//compute size of the whole scene
	double size = sqrt3f(5.0,radius,6.0);

	double radtheta, degtheta;

	radtheta = 2.0 * atan2(size,distance);
	degtheta = (180.0 * radtheta) / PI;

	return degtheta;
}

void reshape(int w, int h)
{
	int size = (w>h)?h:w;
	glViewport((w-size)/2, (h-size)/2, size, size);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//calculate fovy
	double fovy = calculateFovy();
	gluPerspective(fovy, w/h, 0, 1000);
}

int nextModel(){
	int next = currentSegment + 1;
	return (next == totalSegments)? 0: next;
}

float distanceAt(point p1, point p2){
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float dz = p1.z - p2.z;
	return sqrt3f(dx,dy,dz);
}

/* distance between current position and next point is greater than one between current track point and the next point*/
bool isTrespass(){
	int next = nextModel();
	point from = track[currentSegment];
	point to = track[next];
	float d1 = distanceAt(centerPos, from);
	float d2 = distanceAt(to, from);

	return d1 > d2;
}

void rotationMatrix(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(delta,rotationAxis[currentSegment].x,rotationAxis[currentSegment].y,rotationAxis[currentSegment].z);
	glMultMatrixf(acc_matrix);
	glGetFloatv(GL_MODELVIEW_MATRIX,acc_matrix);
	glPopMatrix();
}

void idle(void)
{
	//rotate by constant speed
	theta+=delta;
	if(theta > 360.0)
		theta -= 360.0;

	//translate on direction 
	float offset = (radius * delta* PI)/180;
	centerPos.x = centerPos.x + vectors[currentSegment].x*offset;
	centerPos.y = centerPos.y + vectors[currentSegment].y*offset;
	centerPos.z = centerPos.z + vectors[currentSegment].z*offset;

	if(isTrespass()){
		currentSegment = nextModel();
		centerPos = track[currentSegment];
	}

	//compute accumulated rotation matrix
	rotationMatrix();

	/* display()*/
	glutPostRedisplay();
}

void key(unsigned char key, int x, int y)
{
	if(key == 'b'|| key =='B'){
		begin = true;
		// Start rolling
		glutIdleFunc(idle);
	}
	if(key == 'x') viewer.x-= 1.0;
	if(key == 'X') viewer.x+= 1.0;
	if(key == 'y') viewer.y-= 1.0;
	if(key == 'Y') viewer.y+= 1.0;
	if(key == 'z') viewer.z-= 1.0;
	if(key == 'Z') viewer.z+= 1.0;
}

void mouse(int button, int state, int x, int y){
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP && begin){
		rolling = !rolling;
	}
	if(rolling){
		// Stop rolling
		glutIdleFunc(idle);
	}else{
		glutIdleFunc(NULL);
	}
}

void main(int argc, char **argv)
{
	/*---- Initialize & Open Window ---*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // double-buffering and RGB color
	// mode.
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(30, 30);  // Graphics window position
	glutCreateWindow("Assignment 2"); // Window title is "Rectangle"

	fileReadIn();
	addMenu();

	glutDisplayFunc(display); // Register our display() function as the display 
	// call-back function
	glutReshapeFunc(reshape); // Register our reshape() function as the reshape 
	// call-back function
	glutMouseFunc(mouse);  // for mouse 
	glutKeyboardFunc(key); // for keyboard

	my_init();                // initialize variables

	glutMainLoop();          // Enter the event loop
}
