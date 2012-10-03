/**
CS6533 Assignment 4 : Texture and GPU programming

author:		Zhe Wu
since:		05/18/2011
version:	1.0
base on:    Assignment 4
*/
#define WINDOWS     1 /* Set to 1 for Windows, 0 else */
#define UNIX_LINUX  0 /* Set to 1 for Unix/Linux, 0 else */

#if WINDOWS
#include <windows.h>
#include <GL/glew.h>
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
#include <stdlib.h>

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
void fog_effect(void);
void init_texture(void);


/* Compute distance between two points */
float distanceAt(point p1, point p2);

/* Reset viewer to (7,3,-10) */
void setDefaultView();

//control variables
bool begin = false, rolling = false, shadow = false,wireframe = true, lighting = false, flatshade = true, spotlight = true;
bool blend = false, texture_ground = false, texture_sphere = false, firework = true;

int producefog = 0;

static GLfloat VerticalCoeff[4] = {2.5, 0, 0 ,0};
static GLfloat SlantedCoeff[4] = {1.0, 1.0, 1.0, 0.0};
static GLfloat* CurrentCoeff = VerticalCoeff;
static GLenum CurrentPlane = GL_EYE_LINEAR;
static GLint CurrentGenMode = GL_EYE_LINEAR;

struct PARTICLE
{
	GLfloat velocity[3];
	GLfloat color[3];
};

#define NUMBER_OF_PARTICLE 300

GLuint Program1;

PARTICLE ParticleArray[NUMBER_OF_PARTICLE];

/* global definitions for constants and global image arrays */
#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4*stripeImageWidth];

//texture name of ground
static GLuint texName;

static GLuint sphereTexName;

/*************************************************************
void image_set_up(void):
  generate checkerboard and stripe images. 

* Inside init(), call this function and set up texture objects
  for texture mapping.
  (init() is called from main() before calling glutMainLoop().)
***************************************************************/
void image_set_up(void)
{
	int i, j, c; 
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white */
			{
				c = 255;  
				Image[i][j][0] = (GLubyte) c;
				Image[i][j][1] = (GLubyte) c;
				Image[i][j][2] = (GLubyte) c;
			}
			else  /* green */
			{
				Image[i][j][0] = (GLubyte) 0;
				Image[i][j][1] = (GLubyte) 150;
				Image[i][j][2] = (GLubyte) 0;
			}

			Image[i][j][3] = (GLubyte) 255;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		/*--- Generate 1D stripe image to array stripeImage[] ---*/
		for (j = 0; j < stripeImageWidth; j++) {
			/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
			When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
			*/
			stripeImage[4*j] = (GLubyte)    255;
			stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
			stripeImage[4*j+2] = (GLubyte) 0; 
			stripeImage[4*j+3] = (GLubyte) 255;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		/*----------- End 1D stripe image ----------------*/

} /* end function */

/*----- shader reader -------
 * creates null terminated string from file ---*/
static char* readShaderSource(const char* shaderFile)
{   FILE* fp = fopen(shaderFile, "rb");
    char* buf;
    long size;

    if(fp==NULL) return NULL;
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buf = (char*) malloc((size+1) * sizeof(char)); 
    fread(buf, 1, size, fp);
    buf[size] = '\0'; /* terminate the string with NULL */
    fclose(fp);
    return buf;
}

/*------ Standard GLSL initialization -----------
*** Usage example:
    initShader(&program1, "vmesh.glsl", "fPassThrough.glsl") ---

    ** Create a program object denoted by global var "program1" that uses 
                vertex shader file   "vmesh.glsl" and 
                fragment shader file "fPassThrough.glsl"
---------------------------------------*/
static void initShader(GLuint *programPtr,
                       const GLchar* vShaderFile, const GLchar* fShaderFile)
{   GLint status; 
    GLchar *vSource, *fSource;
    GLuint vShader, fShader; // for vertex and fragment shader handles
    GLuint program;          // for program handle
    GLchar *ebuffer; /* buffer for error messages */
    GLsizei elength; /* length of error message */

    /* read shader files */
    vSource = readShaderSource(vShaderFile);
    if(vSource==NULL)
    {
      printf("Failed to read vertex shader %s\n", vShaderFile);
        exit(EXIT_FAILURE);
    }
    else printf("Successfully read vertex shader %s\n", vShaderFile);

    fSource = readShaderSource(fShaderFile);
    if(fSource==NULL)
    {
      printf("Failed to read fragment shader %s\n", fShaderFile);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully read fragment shader %s\n", fShaderFile);

    /* create program and shader objects */
    vShader = glCreateShader(GL_VERTEX_SHADER);
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    /* attach shaders to the program object */
    glAttachShader(program, vShader); 
    glAttachShader(program, fShader);

    /* read shaders */
    glShaderSource(vShader, 1, (const GLchar**) &vSource, NULL);
    glShaderSource(fShader, 1, (const GLchar**) &fSource, NULL);

    /* compile vertex shader */
    glCompileShader(vShader);

    /* error check */
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to compile vertex shader %s\n", vShaderFile);
       glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetShaderInfoLog(vShader, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
      exit(EXIT_FAILURE);
    }
    else printf("Successfully compiled vertex shader %s\n", vShaderFile);

    /* compile fragment shader */
    glCompileShader(fShader);

    /* error check */
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to compile fragment shader %s\n", fShaderFile);
       glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetShaderInfoLog(fShader, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully compiled fragment shader %s\n", fShaderFile);

    /* link and error check */
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to link program object\n");
       glGetProgramiv(program, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetProgramInfoLog(program, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully linked program object\n\n");

    /*--- Return the created program handle to the 
          "output" function parameter "*programPtr" ---*/
    *programPtr = program; 
}

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

	//without texture on the ground
	glColor3fv(green);      // draw in green.
	glBegin(GL_POLYGON);
	glNormal3f(0,1,0);
	glVertex3f(ground[0].x,ground[0].y,ground[0].z);
	glVertex3f(ground[1].x,ground[1].y,ground[1].z);
	glVertex3f(ground[2].x,ground[2].y,ground[2].z);
	glVertex3f(ground[3].x,ground[3].y,ground[3].z);
	glEnd();

	if(texture_ground){
		init_texture();
		//tecture on the ground
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,texName);

		glColor3fv(green);      // draw in green.
		glBegin(GL_POLYGON);
			glNormal3f(0,1,0);
			glTexCoord2f(0,0);
			glVertex3f(ground[0].x,ground[0].y,ground[0].z);
			glTexCoord2f(0,5);
			glVertex3f(ground[1].x,ground[1].y,ground[1].z);
			glTexCoord2f(6,5);
			glVertex3f(ground[2].x,ground[2].y,ground[2].z);
			glTexCoord2f(6,0);
			glVertex3f(ground[3].x,ground[3].y,ground[3].z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
};

void init_texture(void){
	glGenTextures(1,&texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ImageWidth,ImageHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,Image);

	glGenTextures(1,&sphereTexName);
	glBindTexture(GL_TEXTURE_1D, sphereTexName);

	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
};

void fireworks()
{
	static GLint state = 1;
	static GLuint t0;
	GLint t = (GLuint) glutGet(GLUT_ELAPSED_TIME);

	if(state) {
		t0 = t;
		state = 0;
	}
	if(t % 10000 < 50) {			
		t0 = t;
	}
	
	GLint time_loc, vx_loc, vy_loc, vz_loc;
	GLuint t1;
	glPushMatrix();
    glUseProgram(Program1);
	/* set up parameter location*/
    time_loc = glGetUniformLocation(Program1, "time");
	vx_loc = glGetAttribLocation(Program1, "vx");
	vy_loc = glGetAttribLocation(Program1, "vy");
	vz_loc = glGetAttribLocation(Program1, "vz");

	t1 = (GLint)glutGet(GLUT_ELAPSED_TIME) - t0;
    /* send elapsed time to shaders */
    glUniform1f(time_loc, t1);

	glPointSize(3.0);
	glBegin(GL_POINTS);
	for(int i = 0; i < NUMBER_OF_PARTICLE; i++) {
		glColor3f(ParticleArray[i].color[0], ParticleArray[i].color[1], ParticleArray[i].color[2]);
		glVertexAttrib1f(vx_loc, ParticleArray[i].velocity[0]);
		glVertexAttrib1f(vy_loc, ParticleArray[i].velocity[1]);
		glVertexAttrib1f(vz_loc, ParticleArray[i].velocity[2]);
		glVertex3f(0,0.1,0);
	}
	glEnd();
	glUseProgram(0);
	glPopMatrix();
}

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

	//init texture
	init_texture();

	fog_effect();

	if(firework){
		fireworks();
	}

	//draw sphere
	glPushMatrix();
	draw_sphere();
	glPopMatrix();

	//TODO what if I change the order of depth and color buffer?
	//draw the ground
	glDepthMask(GL_FALSE);
	draw_ground();

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
	glDepthMask(GL_TRUE);
	draw_ground();
	//Draw shadow only if shadow is enable and the viewer is higher than the ground
	if(shadow && viewer.y > 0){
		glPushMatrix();
		draw_shadow();
		glPopMatrix();
	}

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
			if(texture_sphere){
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, CurrentGenMode);
				glTexGenfv(GL_S, CurrentPlane, CurrentCoeff);
				glEnable(GL_TEXTURE_1D);
				glEnable(GL_TEXTURE_GEN_S);
			}
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
	glDisable(GL_TEXTURE_1D);
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
	if(blend){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	for (int a = 0; a < num; a++)
	{
		if(wireframe){
			//draw wireframe shadow
			glBegin(GL_LINE_LOOP);
			glDisable(GL_TEXTURE_1D);
		}else{
			//draw solid shadow
			glBegin(GL_TRIANGLES);
			glEnable(GL_TEXTURE_1D);
		}
		glVertex3f(sphereData[a][0], sphereData[a][1], sphereData[a][2]);
		glVertex3f(sphereData[a][3], sphereData[a][4], sphereData[a][5]);
		glVertex3f(sphereData[a][6], sphereData[a][7], sphereData[a][8]);
		glEnd();
	}
	glDisable(GL_BLEND);
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

void init_fireworks()
{
	int i = 0, err;
	for(i = 0; i < NUMBER_OF_PARTICLE; i++) {
		ParticleArray[i].velocity[0] = 2.0*((rand()%256)/256.0-0.5);
		ParticleArray[i].velocity[1] = 1.2*2.0*((rand()%256)/256.0);
		ParticleArray[i].velocity[2] = 2.0*((rand()%256)/256.0-0.5);
		ParticleArray[i].color[0] = (rand()%256)/256.0;
		ParticleArray[i].color[1] = (rand()%256)/256.0;
		ParticleArray[i].color[2] = (rand()%256)/256.0;
	}

	err = glewInit();

	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
		exit(1);
	}

	initShader(&Program1, "vertex.glsl", "fragment.glsl");
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

	//compute triangle normal
	calculateNormals();
	
	//clear the background color to blue
	glClearColor(0.529,0.807,0.92,0);
	glEnable(GL_DEPTH_TEST);
	//create ground texture
	image_set_up();

	init_fireworks();
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
	if(key == 'v'||key == 'V'){
		CurrentCoeff = VerticalCoeff;
	}
	if(key == 's'||key == 'S'){
		CurrentCoeff = SlantedCoeff;
	}
	if(key == 'o'||key == 'O'){
		CurrentGenMode = GL_OBJECT_LINEAR;
		CurrentPlane = GL_OBJECT_PLANE;
	}
	if(key == 'e'||key == 'E'){
		CurrentGenMode = GL_EYE_LINEAR;
		CurrentPlane = GL_EYE_PLANE;
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

void fog_effect(void){
	switch(producefog)
	{

		case(0):{
			glDisable(GL_FOG);
			break;
		}	
		case(1):{
			GLfloat fogcolor[4]={0.7, 0.7, 0.7, 0.5};
			glEnable(GL_FOG);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogfv(GL_FOG_COLOR, fogcolor);
			glFogf(GL_FOG_START,0.0);
			glFogf(GL_FOG_END, 18.0);

			break;
		}
		case(2):{
			GLfloat fogcolor[4]={0.7, 0.7, 0.7, 0.5};
			glEnable(GL_FOG);
			glFogi(GL_FOG_MODE, GL_EXP);
			glFogfv(GL_FOG_COLOR, fogcolor);
			glFogf(GL_FOG_START,0.0);
			glFogf(GL_FOG_END, 18.0);
			glFogf(GL_FOG_DENSITY, 0.09);

			break;
		}
		case(3):{
			GLfloat fogcolor[4]={0.7, 0.7, 0.7, 0.5};
			glEnable(GL_FOG);
			glFogi(GL_FOG_MODE, GL_EXP2);
			glFogfv(GL_FOG_COLOR, fogcolor);
			glFogf(GL_FOG_START,0.0);
			glFogf(GL_FOG_END, 18.0);
			glFogf(GL_FOG_DENSITY, 0.09);

			break;
		}

	}
};

void fog_menu(int index)
{
	switch (index)
	{
		case(0):{
			producefog = 0;
			break;
		}
		case(1):{
			producefog = 1;	
			break;
		}
		case(2):{
			producefog = 2;	
			break;
		}
		case(3):{
			producefog = 3;	
			break;
		}
	}
	display();
}

void blending_menu(int index){
	blend = (index == 1)?true:false;
}

void texture_menu(int index){
	texture_ground = (index == 1)?true:false;
}

void texture_sphere_menu(int index){
	texture_sphere = (index == 1)?true:false;
}

void firework_menu(int index){
	firework = (index == 1)?true:false;
}

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

	int fog = glutCreateMenu(fog_menu);
	glutAddMenuEntry("No fog", 0);
	glutAddMenuEntry("linear", 1);
	glutAddMenuEntry("exponential", 2);
	glutAddMenuEntry("exponential square", 3);

	int blend = glutCreateMenu(blending_menu);
	glutAddMenuEntry("No", 0);
	glutAddMenuEntry("Yes", 1);

	int texture = glutCreateMenu(texture_menu);
	glutAddMenuEntry("No", 0);
	glutAddMenuEntry("Yes", 1);
	
	int texture_sphere = glutCreateMenu(texture_sphere_menu);
	glutAddMenuEntry("No", 0);
	glutAddMenuEntry("Yes", 1);

	int firework = glutCreateMenu(firework_menu);
	glutAddMenuEntry("No",0);
	glutAddMenuEntry("Yes",1);

	glutCreateMenu(main_menu);
	glutAddMenuEntry("Default View Point", 0);
	glutAddSubMenu("Enable Lighting", lighting);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Wire Frame", 2);
	glutAddSubMenu("Fog Options", fog);
	glutAddSubMenu("Texture Mapping Ground", texture);
	glutAddSubMenu("Texture Mapped Sphere", texture_sphere);
	glutAddSubMenu("Shadow",shadow);

	glutAddSubMenu("Blending Shadow", blend);
	glutAddSubMenu("Shading", shade);
	glutAddSubMenu("Lighting", spotlight);
	glutAddSubMenu("Firework", firework);

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
	glutCreateWindow("Assignment 4"); // Window title is "Rectangle"

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