/**
CS6533 Assignment 1 : Bresenham's algorithm and Animation

author:		Zhe Wu
since:		02/21/2011
version:	1.0

- Implementation of  Bresenham's algorithm for question c¡¢d¡¢e.

- Put the sample file in C:. It should look like "C:\\sample.txt"

- Run the executable file, and choose following options in command line;

	1 : Draw circle from keyboard input
	2 : Draw circles from an Input File
	3 : Draw animation of growing circles based on question d
	4 : Quit
	    
You can press '3' to make animation while executing
		press '2' to make static circles
		press 's' to switch between solid circles and empty circles

You can also press the key UP to speed up and key DOWN to slow down the animation of option 3

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600
#define MAX_SPEED     100
#define MIN_SPEED     800

/*define a point */
struct Point{
	int x;//x coordinate
	int y;//y corrdinate
};

struct Circle {
	int center_x;
	int center_y;
	int radius;
};

//circle data storage
vector<Circle> data;

//scale factor, being used to transform the world coordinates to screen coordinates
float scaling = 1.0;

//transform from world coordinate to screen coordinates if true
bool transform = false;

//animation spreed, circles grow slower while speed value gets larger
int speed = 300;

//indicate current frame of animation, always less than or equal to speed
int frame = speed;

//draw solid circles if this value is true; empty circles otherwise
bool isSolid = false;

/* Function to draw _every_ frame. */
void display(void);

/* This function computes the next frame value for animation. It is only registered for animation */
void idle(void);

/* Function to handle keyboard events */
void keyboard(unsigned char key, int x, int y);

/* Control the animation speed */
void pressKey(int key, int x, int y);

/* Receive commands from command line */
void command(void);

/* myinit(): Set up attributes and viewing */
void myinit(void);

/* Function to handle file input; modification may be needed */
void file_in(void);

/* transform world coordinate to screen coordinate */
void transCordinate(void);

/* draw a circle centered at (x,y) with radius r*/
void drawCircle(const int& x, const int& y, const int& r);

/* calculate points of circle boundary*/
vector<Point> calculateCircle(const int& r);

/* Compute next point for circle, based on Bresenham's algorithm*/
Point chooseNextPoint(Point p, int& d,int r);

/* draw the eight symmetric points in the eight region */
void circlePoint(const vector<Point>& circlePoints, const int& center_x, const int& center_y);

/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    /* Use both double buffering and Z buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowPosition(XOFF, YOFF);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("CS6533 Assignment 1");

	command();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(pressKey);
	// transform world coordinate to screen coordinate
	transCordinate();

    myinit();
    glutMainLoop();
}

/* Display information */
void showOptions(){
	cout<<"CS6533 Assignment 01\n";
	cout<<"Please input 1, 2, 3 or 4 :\n";
	cout<<"1 : Draw circle from keyboard input \n";
	cout<<"2 : Draw circles from an Input File \n";
	cout<<"    you can press '3' to make animation while executing\n";
	cout<<"3 : Draw animation of growing circles based on question d\n";
	cout<<"    you can press 's' to make circles solid while executing\n";
	cout<<"4 : Quit\n";
}

/* Receive commands from command line */
void command(void){
	char command = -1;
	do{
		showOptions();
		cin.sync();
		//receive command
		cin>>command;

		if(command == '1'){
			//Question c
			Circle circle;
			cout <<"Please input x-coordinate for circle center\n";
			cin >> circle.center_x;
			cout <<"Please input y-coordinate for circle center\n";
			cin >> circle.center_y;
			cout <<"Please input radius for circle\n";
			cin >> circle.radius;

			data.push_back(circle);

			//do not need to transform world coordinates to screen coordinates
			transform = false;
		}else if(command == '2'||command == '3'){
			//Question d and e
			//read input file
			file_in();

			if(command == '3'){
				//register idle function to make animation
				glutIdleFunc(idle);
			}
			//transform world coordinates to screen coordinates
			transform = true;
		}else if(command == '4'){
			cout <<"Bye bye! Have nice day!"<<endl;
			exit(0);
		}else {
			cout <<"Incorrect command! Please try again.\n"<<endl;
			command = -1;
		}
	} while (command == -1);
}

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(void)
{
	const int MAX_FILE_LEN = 1000;
	char* filePath = new char[MAX_FILE_LEN];
	cout << "Please enter data file path, or press ENTER to use the sample file(C:\\sample.txt) " << endl;
	cin.sync();
	cin.getline(filePath,MAX_FILE_LEN);
	if(strlen(filePath) == 0){
		cout <<"No input file '"<< filePath <<"'. Use sample data."<<endl;
		filePath = "C:\\sample.txt";
	}
	ifstream file(filePath);
	if(file.fail()){
		cout <<"Cannot open file "<<filePath<<endl;
		exit(0);
	}
	int count = -1;
	file >> count;
	cout <<"Total number of record: "<<count<<endl;
	while(!file.eof()){
		Circle circle;
		file >> circle.center_x;
		file >> circle.center_y;
		file >> circle.radius;
		data.push_back(circle);
		cout << "circle with radius "<<circle.radius<<" centered at ("<<circle.center_x<<","<<circle.center_y<<")"<<endl;
	}
}

/* Transform world coordinate to screen coordinate */
void transCordinate(void){
	//find the maximal range on x-axis and y-axis of input data
	int x_max = 0, y_max = 0;
	for(int i=0;i<data.size();i++){
		int range_x = abs(data[i].center_x)+data[i].radius;
		int range_y = abs(data[i].center_y)+data[i].radius;
		//find max x-axis value
		x_max = max(range_x,x_max);
		//find max y-axis value
		y_max = max(range_y,y_max);
	}

	//compute scalar to map the world coordinates to the screen coordinates
	if(x_max >= y_max&&x_max > WINDOW_WIDTH/2){
		//scale by x axis
		scaling = 1.0 * WINDOW_WIDTH / (2 * x_max);
		transform = true;
	}else if(x_max < y_max&&y_max > WINDOW_HEIGHT/2){
		//scale by y axis
		scaling = 1.0 * WINDOW_HEIGHT / (2 * y_max);
		transform = true;
	}

	//mapping coordinates
	for(int i=0;i<data.size();i++){
		//adjust the size of circles
		data[i].center_x *= scaling;
		data[i].center_y *= scaling;
		data[i].radius *= scaling;
		
		//do not need mapping for question c
		if(transform){
			//map the origin of world coordinates to the center of OpenGL window
			data[i].center_x +=WINDOW_WIDTH/2;
			data[i].center_y +=WINDOW_HEIGHT/2;
		}
	}
}

/*---------------------------------------------------------------------
display(): This function is called once for _every_ frame. 
---------------------------------------------------------------------*/
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 0.84, 0);              /* draw in golden yellow */
	glPointSize(1.0);                     /* size of each point */
	for(int i=0;i<data.size();i++){
		//compute circle radius based on current frame
		int radius = data[i].radius * frame/speed;
		//draw a circle
		drawCircle(data[i].center_x,data[i].center_y,radius);
	}
	glFlush();                            /* render graphics */
	glutSwapBuffers();                    /* swap buffers */
}

/*---------------------------------------------------------------------
myinit(): Set up attributes and viewing
---------------------------------------------------------------------*/
void myinit(void)
{
  glClearColor(0.0, 0.0, 0.92, 0.0);    /* blue background*/

  /* set up viewing */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
  glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------
draw_circle(int x, int y, int r): draw a circle centered at (x,y) with radius r
---------------------------------------------------------------------*/
void drawCircle(const int& x, const int& y, const int& r){
	//valid radius value, radius must be positive
	int radius = (r<=0) ? 1 : r;
	vector<Point> circlePoints = calculateCircle(radius);
	//symetrically draw the mirrior points
	circlePoint(circlePoints,x,y);
}

/* Calculate points of circle boundary*/
vector<Point> calculateCircle(const int& radius){
	vector<Point> circlePoints;
	//Dstart vlaue
	int difference = -4*radius+1;
	//first p point
	Point p;
	p.x = radius;
	p.y = 0;
	circlePoints.push_back(p);

	//only need to consider the region from (radius,0) to (radius//sqrt(2),radius//sqrt(2))
	while(p.x > p.y){
		p = chooseNextPoint(p,difference,radius);
		circlePoints.push_back(p);
	}
	return circlePoints;
}

/* Compute next point for circle, based on Bresenham's algorithm*/
Point chooseNextPoint(Point currentPoint, int& difference,int radius){
	Point nextPoint;
	// choose x coordinate by D
	if(difference <= 0){
		difference = difference + 8 * currentPoint.y + 4;
	}else {
		difference = difference - 8*(currentPoint.x - currentPoint.y) + 4;
	}
	if(difference <= 0){
		nextPoint.x = currentPoint.x;
		nextPoint.y = currentPoint.y+1;
	}else {
		nextPoint.x = currentPoint.x-1;
		nextPoint.y = currentPoint.y+1;
	}
	return nextPoint;
}

/* Draw the eight symmetric points in the eight regions corresponding to each point*/
void circlePoint(const vector<Point>& circlePoints, const int& center_x, const int& center_y){
	if(isSolid){
		//draw solid circles
		glBegin(GL_LINES);
		for(int i=0;i<circlePoints.size();i++){
			glVertex2i(center_x + circlePoints[i].x,center_y + circlePoints[i].y);
			glVertex2i(center_x - circlePoints[i].x,center_y + circlePoints[i].y);
			glVertex2i(center_x + circlePoints[i].y,center_y + circlePoints[i].x);
			glVertex2i(center_x - circlePoints[i].y,center_y + circlePoints[i].x);
			glVertex2i(center_x + circlePoints[i].y,center_y - circlePoints[i].x);
			glVertex2i(center_x - circlePoints[i].y,center_y - circlePoints[i].x);
			glVertex2i(center_x - circlePoints[i].x,center_y - circlePoints[i].y);
			glVertex2i(center_x + circlePoints[i].x,center_y - circlePoints[i].y);
		}
		glEnd();
	}else {
		//draw empty ones
		glBegin(GL_POINTS);
		for(int i=0;i<circlePoints.size();i++){
			glVertex2i(center_x + circlePoints[i].x,center_y + circlePoints[i].y);
			glVertex2i(center_x + circlePoints[i].y,center_y + circlePoints[i].x);
			glVertex2i(center_x - circlePoints[i].x,center_y + circlePoints[i].y);
			glVertex2i(center_x + circlePoints[i].y,center_y - circlePoints[i].x);
			glVertex2i(center_x - circlePoints[i].x,center_y - circlePoints[i].y);
			glVertex2i(center_x - circlePoints[i].y,center_y - circlePoints[i].x);
			glVertex2i(center_x + circlePoints[i].x,center_y - circlePoints[i].y);
			glVertex2i(center_x - circlePoints[i].y,center_y + circlePoints[i].x);
		}
		glEnd();
	}
}

/* keyboard events:
   key '2': draw static circles
   key '3': draw animation circles
   key 's': switch between solid and empty circles
*/
void keyboard(unsigned char key, int x, int y){
	switch(key){
		case '2':
			//draw static circles
			glutIdleFunc(NULL);
			break;
		case '3':
			//draw animation circles
			glutIdleFunc(idle);
			break;
		case '4':
			exit(0);
			break;
		case 's':
			//make the circles solid
			isSolid = !isSolid;
			break;
	}
}

/* Control the animation speed*/
void pressKey(int key, int x, int y){
	switch(key){
		case GLUT_KEY_DOWN:
			//slow down
			speed = (speed >= MIN_SPEED)? speed:speed+50;
			break;
		case GLUT_KEY_UP:
			//speed up
			speed = (speed <= MAX_SPEED)? speed:speed-50;
			break;
	}
}

/* Compute next frame*/
void idle(void){
	if(frame > speed){
		frame = 1;	
	}
	frame++;
	glutPostRedisplay();
}