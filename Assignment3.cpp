/**
CS6533 Assignment 3 : Lighting and Shading

author:		Zhe Wu
since:		04/27/2011
version:	1.0
base on:    Assignment 2

- Projecting the shadow of rotating sphere to the ground.
	Using "decal" technique.

- Add directional lighting as described in question (c).

- Implement flat shading and smooth shading.

- Add spot light and point light as described in question (d).

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

#define DEFAULT_SPHERE_PATH "C:\\sphere.1024"

struct point {
	GLfloat x, y, z;
};

typedef point vector;

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

/* Normals of triangles on sphere*/
point* tri_normals;

int data_count = -1,num,polygon_n;

/* Theta: the accumulated rotating angle */
/* Delta: the angle rotating each time */
GLfloat theta = 0.0, delta = 5;

/* Accumulated Matrix */
GLfloat acc_matrix[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

/*Shadow Matrix*/
GLfloat shadow_matrix[16]={12,0,0,0,14,0,3,-1,0,0,12,0,0,0,0,12};

/* Colors and position for shading effect*/
GLfloat light_pos[] = {-14.0 ,12.0 ,-3.0 , 1.0};
GLfloat light_dir[] = {8.0,-12.0,-1.5,0};

GLfloat global_ambient[]={1,1,1,1};
GLfloat light_ambient[]={0,0,0,1};
GLfloat light_diffuse[]={1,1,1,1};
GLfloat light_specular[]={1,1,1,1};
GLfloat green[]={0.0, 1.0, 0.0,1.0};
GLfloat golden[] = {1.0, 0.84, 0.0, 1.0};
GLfloat mat_shiness = 125.0;

void display(void);
void my_init(void);
void reshape(int w, int h);
void idle(void);
void key(unsigned char key, int x, int y);
void mouse(int btn,int state, int x,int y);
void draw_shadow(void);
void draw_sphere(void);
void init_lighting(void);
void draw_axes(void);
void init_spotlight(void);
void init_lighting(void);

/* Compute distance between two points */
float distanceAt(point p1, point p2);

/* Reset viewer to (7,3,-10) */
void setDefaultView();

//control variables
bool begin = false, rolling = false, shadow = false,wireframe = true, lighting = false, flatshade = true, spotlight = true;

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

void draw_ground(){
	//if lighting is on, draw ground with shade
	if(lighting){
		glEnable(GL_LIGHTING);
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,green);
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_ambient);
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,light_specular);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,mat_shiness);
	}

	glColor3fv(green);      // draw in green.
	glBegin(GL_POLYGON);
	glNormal3f(0,1,0);
	glVertex3f(ground[0].x,ground[0].y,ground[0].z);
	glVertex3f(ground[1].x,ground[1].y,ground[1].z);
	glVertex3f(ground[2].x,ground[2].y,ground[2].z);
	glVertex3f(ground[3].x,ground[3].y,ground[3].z);
	glEnd();
};

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  // clear frame buffer (also called the color buffer)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//init directional light
	//init_lighting();	

	gluLookAt(viewer.x,viewer.y,viewer.z,  0,0,0,  0,1,0);

	//init spotlight
	init_spotlight();

	//draw sphere
	glPushMatrix();
	draw_sphere();
	glPopMatrix();

	//TODO what if I change the order of depth and color buffer?
	//draw the ground
	glDepthMask(GL_FALSE);
	draw_ground();
	glDepthMask(GL_TRUE);

	//draw the axes
	draw_axes();

	//Draw shadow only if shadow is enable and the viewer is higher than the ground
	if(shadow && viewer.y > 0){
		glPushMatrix();
		draw_shadow();
		glPopMatrix();
	}

	//draw the ground again with z-buffer enabled
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	draw_ground();
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	glFlush();         // Render (draw) the object
	glutSwapBuffers(); // Swap buffers in double buffering.
}

void draw_axes(){
	//glLineWidth(2.0);
	glDisable(GL_LIGHTING);

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
};

/* draw sphere by drawing triangles */
void draw_sphere(){
	glTranslatef(centerPos.x,centerPos.y,centerPos.z);
	//add rotation here
	//Rotate the sphere around vector 8i+7k
	glMultMatrixf(acc_matrix);

	if(lighting){
		glEnable(GL_LIGHTING);
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,golden);
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,golden);
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_ambient);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,mat_shiness);
	}
	glColor3fv(golden);      // draw in golden yellow.
	for (int a = 0; a < num; a++)
	{
		if(wireframe){
			//Do not need shading when drawing wireframe
			glDisable(GL_LIGHTING);
			//draw wireframe sphere
			glBegin(GL_LINE_LOOP);
		}else{
			//draw solid sphere
			glBegin(GL_TRIANGLES);
		}
		if(flatshade){
			glNormal3f(tri_normals[a].x,tri_normals[a].y,tri_normals[a].z);
			glVertex3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
			glVertex3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
			glVertex3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
		} else {
			glNormal3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
			glVertex3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
			glNormal3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
			glVertex3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
			glNormal3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
			glVertex3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
		}

		glEnd();
	}
};

void draw_shadow(){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(viewer.x,viewer.y,viewer.z,  0,0,0,  0,1,0);

	//draw shadow on the ground
	glTranslatef(centerPos.x+1,0,centerPos.z+1);
	glMultMatrixf(shadow_matrix);
	//glMultMatrixf(acc_matrix);
	glColor4f(0.25,0.25,0.25,0.65);      // draw in black.

	//TODO we don't want lighting on shadow, but we can try the effect later
	glDisable(GL_LIGHTING);
	for (int a = 0; a < num; a++)
	{
		if(wireframe){
			//draw wireframe shadow
			glBegin(GL_LINE_LOOP);
		}else{
			//draw solid shadow
			glBegin(GL_TRIANGLES);
		}
		glVertex3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
		glVertex3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
		glVertex3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
		glEnd();
	}
};

vector calculateDirection(point from, point to){
	vector v;
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

vector crossProduct(vector u, vector v){
	vector n;
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

vector normalize(vector v){
	//convert v to unit-length
	float d = sqrt3f(v.x,v.y,v.z);
	v.x = v.x/d;
	v.y = v.y/d;
	v.z = v.z/d;

	return v;
};

/* compute normals for triangles */
void calculateNormals(){
	//calculate the normal of triangles
	tri_normals = new point[num];
	for (int a = 0; a < num; a++)
	{
		point p1, p2, p3;
		p1.x = sphereData[a][0];
		p1.y = sphereData[a][1];
		p1.z = sphereData[a][2];
		p2.x = sphereData[a][3];
		p2.y = sphereData[a][4];
		p2.z = sphereData[a][5];
		p3.x = sphereData[a][6];
		p3.y = sphereData[a][7];
		p3.z = sphereData[a][8];

		vector v1 = calculateDirection(p1,p2);
		vector v2 = calculateDirection(p2,p3);

		vector n = crossProduct(v1,v2);
		tri_normals[a] = normalize(n);
	}
};

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

	//compute triangle normal
	calculateNormals();
	
	//clear the background color to blue
	glClearColor(0.529,0.807,0.92,0);
	glEnable(GL_DEPTH_TEST);
}

void init_spotlight(){
	//init spot light
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1,GL_POSITION,light_pos);
	glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION,light_dir);

	glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT1,GL_SPECULAR,light_specular);
	glLightfv(GL_LIGHT1,GL_AMBIENT,light_ambient);

	if(spotlight){
		glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,20);
	}else {
		glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,180);
	}
	glLightf(GL_LIGHT1,GL_SPOT_EXPONENT,5);
	glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION,2.0);
	glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,0.01);
	glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION,0.001);
};

void init_lighting(){
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

	//global ambient light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,global_ambient);
	//use flat shading as default
	if(flatshade){
		glShadeModel(GL_FLAT);
	}else{
		glShadeModel(GL_SMOOTH);
	}
};

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
	gluPerspective(fovy, w/h, 1, 100);
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
	Sleep(10);
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

/* set to default VRP*/
void setDefaultView(){
	viewer.x = 7.0;
	viewer.y = 3.0;
	viewer.z = -10.0;
}

/* Release program and exit*/
void quit(){
	delete [] vectors;
	delete [] rotationAxis;
	delete [] tri_normals;

	for(int i=0;i<num;i++){
		delete [] sphereData[i];
	}

	delete [] sphereData;

	exit(1);
}

/* Main menu handler: 
1) Reset default view point
2) Quit
3) Switch between wireframe and solid sphere
*/
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
	case(2):
		{
		wireframe = !wireframe;
		break;
		}
	}
	display();
};

/* shadow menu handler
1) Turn on shadow
2) Turn off shadow
*/
void shadow_menu(int index){
	shadow = (index == 1)?false:true;
	display();
};

/* shade menu handler
1) Flat shade
2) Smooth shade
*/
void shade_menu(int index){
	flatshade = (index == 1)?true:false;
	if(flatshade){
		glShadeModel(GL_FLAT);
	}else {
		glShadeModel(GL_SMOOTH);
	}
	display();
};

/* lighting menu handler
1) Turn on lighting effect
2) Turn off lighting effect
*/
void lighting_menu(int index){
	lighting = (index == 1)?false:true;
	display();
};

/* spotlight menu handler
1) Spotlight
2) Point light
*/
void spotlight_menu(int index){
	spotlight = (index == 1)?true:false;
	display();
};

/* Add menu to mouse left button*/
void addMenu(){
	int shadow = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shade = glutCreateMenu(shade_menu);
	glutAddMenuEntry("FLAT", 1);
	glutAddMenuEntry("SMOOTH", 2);

	int lighting = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int spotlight = glutCreateMenu(spotlight_menu);
	glutAddMenuEntry("Spot light", 1);
	glutAddMenuEntry("Point light", 2);
	
	glutCreateMenu(main_menu);
	glutAddMenuEntry("Default View Point", 0);
	glutAddSubMenu("Enable Lighting", lighting);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Wire Frame", 2);
	glutAddMenuEntry("Fog Options", NULL);
	glutAddMenuEntry("Texture Mapping", NULL);
	glutAddSubMenu("Shadow",shadow);

	glutAddMenuEntry("Blending Shadow", NULL);
	glutAddSubMenu("Shading", shade);
	glutAddSubMenu("Lighting", spotlight);
	glutAddMenuEntry("Firework", NULL);

	glutAttachMenu(GLUT_LEFT_BUTTON);
}

void main(int argc, char **argv)
{
	/*---- Initialize & Open Window ---*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // double-buffering and RGB color
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
