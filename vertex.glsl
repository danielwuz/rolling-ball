/****************************
 Filename: vertex.glsl
 This is a vertex shader. It varies the height of each mesh vertex
 sinusoidally using the elapsed-time parameter "time".
*****************************/

uniform float time; /* elapsed time in milliseconds; 0.001*time to get
                       elapsed time in seconds */
attribute float vx;

attribute float vy;

attribute float vz;

varying float pos_y;

void main()
{
	vec4 t = gl_Vertex; /* current vertex coordinates given by glVertex() */

	
	t.x = 0.001*vx*time;
	t.y = 0.1 + 0.001*vy*time + 0.5*(-0.00000049)*time*time;
	t.x = 0.001*vz*time;
	
	pos_y = t.y;
	gl_Position = gl_ModelViewProjectionMatrix * t;
	gl_FrontColor = gl_Color;
}
