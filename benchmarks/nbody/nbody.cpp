#include "float4.hpp"
#include "simulation.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>

//#define GRAPHICS

#ifdef GRAPHICS
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <GL/glext.h>
#include <GL/glx.h>
#endif

#include <runtime.h>

double ratio=-1.0f;
int particlesnumber;
float bound, dt, duration, runtime, *potential;
int mouse_old_x, mouse_old_y, mouse_buttons, rotate_x, rotate_y, translate_z, steps_counter, max_steps;
cl_float4* pos, *vel, *force, *color;
int cubeon;

int iDivUp(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

static unsigned long int next = 1;

int _rand(void) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next/65536) % 32768;
}

cl_float4 f4(float x, float y, float z, float w) {
	cl_float4 a;
	a.x = x;
	a.y = y;
	a.z = z;
	a.w = w;
	return a;
}

void _srand(unsigned int seed)
{
	next = seed;
}

#ifdef GRAPHICS
void keyboardEvents(unsigned char key, int x, int y) {
	switch(key) {
		case 'Q':
		case 'q':
			glutLeaveMainLoop();
			break;
		case 'C':
		case 'c':
			cubeon = (cubeon)?0:1;
			break;
	}
}


void mouseEvents(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mouse_buttons |= 1<<button;
	} else if (state == GLUT_UP) {
		mouse_buttons = 0;
	}

	mouse_old_x = x;
	mouse_old_y = y;
}

void appMotion(int x, int y) {

	float dx, dy;
	dx = x - mouse_old_x;
	dy = y - mouse_old_y;

	if (mouse_buttons & 1) {
		rotate_x += dy * 0.2;
		rotate_y += dx * 0.2;
	} else if (mouse_buttons & 4) {
		translate_z += dy * 0.1;
	}

	mouse_old_x = x;
	mouse_old_y = y;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, translate_z);
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);
}


GLuint pos_gl, col_gl;

void drawEvent()
{
	size_t globalwork[2], localwork[2];
	localwork[0] = 512;
	localwork[1] = 1;
	globalwork[0] = iDivUp(particlesnumber,localwork[0]) *localwork[0];
	globalwork[1] = 1;

	update(pos,color, force, vel, potential, bound, dt, particlesnumber, ratio);

	if (steps_counter%100 == 0)
	{
		double E_kin = 0.0, E_V = 0.0;
		for (int i=0; i < particlesnumber; i++)
		{
			E_kin += 0.5f * ((double)vel[i].x*vel[i].x + (double)vel[i].y*vel[i].y + (double)vel[i].z*vel[i].z);
			E_V += potential[i];
		}

		printf("System's Total Energy:%g\n",E_kin+E_V);
	}
	steps_counter++;

	glBindBuffer(GL_ARRAY_BUFFER, pos_gl);
	glBufferSubData(GL_ARRAY_BUFFER, 0,particlesnumber*sizeof(cl_float4), pos);
	glBindBuffer(GL_ARRAY_BUFFER, col_gl);
	glBufferSubData(GL_ARRAY_BUFFER, 0,particlesnumber*sizeof(cl_float4), color);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (cubeon)
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		glutSolidCube(2*bound);
	}

	// Render the particles from VBOs.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.);

	// Color buffer.
	glBindBuffer(GL_ARRAY_BUFFER, col_gl);
	glColorPointer(4, GL_FLOAT, 0, 0);

	// Vertex buffer.
	glBindBuffer(GL_ARRAY_BUFFER, pos_gl);
	glVertexPointer(4, GL_FLOAT, 0, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisableClientState(GL_NORMAL_ARRAY);

	glDrawArrays(GL_POINTS, 0, particlesnumber);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glutSwapBuffers();

	if (steps_counter>max_steps)
		glutLeaveMainLoop();
}

void timerCB(int ms) {

	glutTimerFunc(ms, timerCB, ms);
	glutPostRedisplay();
}

void InitializeOpenGL(int argc, char *argv[]) {
	int wWidth = 600, wHeight = 600;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(wWidth, wHeight);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 -
			wWidth/2,
			glutGet(GLUT_SCREEN_HEIGHT)/2 -
			wHeight/2);

	char windowTitle[100];
	sprintf(windowTitle,"Centaurus N-Body Simulation (bound:%g, dt:%g, particles:%d)", bound, dt, particlesnumber);
	glutCreateWindow(windowTitle);

	glutDisplayFunc(drawEvent);
	glutKeyboardFunc(keyboardEvents);
	glutMouseFunc(mouseEvents);
	glutMotionFunc(appMotion);

	glewInit();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	// Viewport.
	glViewport(0, 0, wWidth, wHeight);

	// Projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (GLfloat)wWidth/(GLfloat)wHeight, 0.1, 3000.0);

	// Set view matrix.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -2.7f * 300.0f);

	// Set a minimum time between frames.
	glutTimerFunc(50, timerCB, 50);  

}
#endif

int main(int argc, char *argv[])
{	
	char temp[30];
	int graphics = 0, option = 0;
	int userParticles=-1, userIters=-1;
	int threads = -1;

	cubeon = 0;
	while ((option = getopt(argc, argv,"gn:?m:?t:r:")) != -1) {
		switch (option) {
			case 'g' : 
				graphics = 1;
				break;
			case 'n' :
				userParticles = atoi(optarg);
				break;
			case 'm' :
				userIters = atoi(optarg);
				break;
			case 't':
				threads = atoi(optarg);
				break;
			case 'r':
				ratio = atof(optarg);
				break;
		}
	}

	assert(threads != -1 );
	assert(ratio>=0.0);


	FILE * fp = fopen("input", "r");
	int ret;
	ret = fscanf(fp, "%s %g", temp, &bound);
	assert(ret==2);
	printf("%s:%g\n",temp,bound);

	ret = fscanf(fp, "%s %g", temp, &dt);
	assert(ret==2);
	printf("%s:%g\n",temp,dt);

	ret=fscanf(fp, "%s %d", temp, &particlesnumber);
	assert(ret==2);
	if ( userParticles !=-1 )
		particlesnumber = userParticles;
	printf("%s:%d\n",temp,particlesnumber);

	ret = fscanf(fp, "%s %g", temp, &duration);
	assert(ret==2);
	printf("%s:%g\n",temp,duration);

	runtime = 0.0f;
	steps_counter = 0;
	translate_z = -2.7f * bound;
	mouse_buttons = 0;

	pos   = (cl_float4*)calloc(particlesnumber,sizeof(cl_float4));
	force = (cl_float4*)calloc(particlesnumber,sizeof(cl_float4));
	vel   = (cl_float4*)calloc(particlesnumber,sizeof(cl_float4));
	color   = (cl_float4*)calloc(particlesnumber,sizeof(cl_float4));
	potential = (float*)calloc(particlesnumber,sizeof(float));

	size_t globalwork[2], localwork[2];

	localwork[0] = 512;
	localwork[1] = 1;
	globalwork[0] = iDivUp(particlesnumber,localwork[0]) *localwork[0];
	globalwork[1] = 1;

	float max_pos = 2.0f*bound;
	float min_pos = bound;
	float max_vel = max_pos;
	float min_vel = min_pos;

	_srand (100);

	for(int i = 0; i < particlesnumber; i++) {

		float x = ((float)_rand())/((float)(32767))*max_pos - min_pos;
		float z = ((float)_rand())/((float)(32767))*max_pos - min_pos;
		float y = ((float)_rand())/((float)(32767))*max_pos - min_pos;
		pos[i] = f4(x, y, z, 1.f);

		x = ((float)_rand())/((float)(32767))*max_vel - min_vel;
		z = ((float)_rand())/((float)(32767))*max_vel - min_vel;
		y = ((float)_rand())/((float)(32767))*max_vel - min_vel;
		vel[i] = f4(x, y, z, 0.f);

		force[i] = f4(0.f, 0.f, 0.f, 0.f);

		color[i] = f4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	
	init_system(threads);
	
	if ( userIters == -1 )
	{
		max_steps = (int)duration/dt;
	}
	else
	{
		max_steps = userIters;
	}
	#ifdef GRAPHICS
	if (graphics)
	{
		InitializeOpenGL(argc,argv);

		glGenBuffers(1, &pos_gl);
		glBindBuffer(GL_ARRAY_BUFFER, pos_gl);
		glBufferData(GL_ARRAY_BUFFER,  particlesnumber*sizeof(cl_float4), NULL, GL_STATIC_DRAW);        	
		glBufferSubData(GL_ARRAY_BUFFER, 0,particlesnumber*sizeof(cl_float4), pos);

		glGenBuffers(1, &col_gl);
		glBindBuffer(GL_ARRAY_BUFFER, col_gl);
		glBufferData(GL_ARRAY_BUFFER,  particlesnumber*sizeof(cl_float4), NULL, GL_STATIC_DRAW);        	
		glBufferSubData(GL_ARRAY_BUFFER, 0,particlesnumber*sizeof(cl_float4), color);

		update(pos,color, force, vel, potential, bound, dt, particlesnumber, ratio);
		steps_counter++;

		glutMainLoop();
	}
	else
	#endif
	{
		update(pos, color, force, vel, potential, bound, dt, particlesnumber, ratio);

		for (int run=1; run<=max_steps; run++)
		{
			update(pos, color, force, vel, potential, bound, dt, particlesnumber, ratio);
		}
	}

	shutdown_system();

	double E_kin = 0.0, E_V = 0.0;
	for (int i=0; i < particlesnumber; i++)
	{
		E_kin += 0.5f * ((double)vel[i].x*vel[i].x + (double)vel[i].y*vel[i].y + (double)vel[i].z*vel[i].z);
		E_V += potential[i];
	}
	
	
	printf("System's Total Energy:%.32g\n",E_kin+E_V);


	
	std::ofstream output;
	E_kin += E_V;
	output.open("positions", std::ios::binary);
	output.write((const char*)&particlesnumber, sizeof(particlesnumber));
	output.write((const char*)&E_V, sizeof(E_V));

	for ( int i=0; i<particlesnumber; ++i )
	{
		output.write((const char*)&pos[i].x, sizeof(float));
		output.write((const char*)&pos[i].y, sizeof(float));
		output.write((const char*)&pos[i].z, sizeof(float));

		// std::cout << pos[i].x << " "  << pos[i].y << " " << pos[i].z << std::endl;
	}

	output.close();

	return 0;

}

