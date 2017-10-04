//Assignment 2
//@authors: Andrew Hwang and Timothy Kuo
//

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#define ON 1
#define OFF 0
#define LEFT 0							/*left-mouse button*/
#define RIGHT 2							/*right-mouse button*/
#define PI 3.1415926535f
#define SENSITIVITY 100.0f				/*sensitivity of the rate that the camera's point of view changes*/

//Global variables
int window_width, window_height;
int PERSPECTIVE = ON;
int displayCoordAxes = ON;
int displayObj = ON;
float radius = 5.0f;					/*distance the camera is from the origin of the world coordinate*/
float prevRadius = radius;
// Persepective angles
float theta = -PI / 2;					/*angle between x and z axes*/
float phi = -PI / 2;						/*angle between the y and combination(x+z) axes*/
float prevPhi = phi;
// Orthogonal angles
float alpha = -PI / 2;					/*angle between x and z axes*/
float omega = -PI / 2;					/*angle between the y and combination(x+z) axes*/
float prevOmega = omega;
float cameraX, cameraY, cameraZ;
float clickX, clickY, clickedButton;	/*gloabl vaiables saving the data that is collected from clicking a button on the mouse*/

typedef struct _point {
	float x, y, z;
} point;

typedef struct _faceStruct {
	int v1, v2, v3;
	int n1, n2, n3;
} faceStruct;

int verts, faces, norms;
point *vertList, *normList;
faceStruct *faceList;

// It can read *very* simple obj files
void meshReader(char *filename, int sign) {
	float x, y, z, len;
	int i;
	char letter;
	point v1, v2, crossP;
	int ix, iy, iz;
	int *normCount;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Cannot open %s\n!", filename);
		exit(0);
	}

	// Count the number of vertices and faces
	while (!feof(fp)) {
		fscanf(fp, "%c %f %f %f\n", &letter, &x, &y, &z);
		if (letter == 'v') {
			verts++;
		}
		else {
			faces++;
		}
	}

	fclose(fp);

	printf("verts : %d\n", verts);
	printf("faces : %d\n", faces);

	// Dynamic allocation of vetex and face lists
	faceList = (faceStruct *)malloc(sizeof(faceStruct)*faces);
	vertList = (point *)malloc(sizeof(point)*verts);
	normList = (point *)malloc(sizeof(point)*verts);

	fp = fopen(filename, "r");

	// Read the vertices
	for (i = 0; i < verts; i++) {
		fscanf(fp, "%c %f %f %f\n", &letter, &x, &y, &z);
		vertList[i].x = x;
		vertList[i].y = y;
		vertList[i].z = z;
	}

	// Read the faces
	for (i = 0; i < faces; i++) {
		fscanf(fp, "%c %d %d %d\n", &letter, &ix, &iy, &iz);
		faceList[i].v1 = ix - 1;
		faceList[i].v2 = iy - 1;
		faceList[i].v3 = iz - 1;
	}
	fclose(fp);

	normCount = (int *)malloc(sizeof(int)*verts);
	for (i = 0; i < verts; i++) {
		normList[i].x = normList[i].y = normList[i].z = 0.0;
		normCount[i] = 0;
	}

	for (i = 0; i < faces; i++) {
		v1.x = vertList[faceList[i].v2].x - vertList[faceList[i].v1].x;
		v1.y = vertList[faceList[i].v2].y - vertList[faceList[i].v1].y;
		v1.z = vertList[faceList[i].v2].z - vertList[faceList[i].v1].z;
		v2.x = vertList[faceList[i].v3].x - vertList[faceList[i].v2].x;
		v2.y = vertList[faceList[i].v3].y - vertList[faceList[i].v2].y;
		v2.z = vertList[faceList[i].v3].z - vertList[faceList[i].v2].z;

		crossP.x = v1.y*v2.z - v1.z*v2.y;
		crossP.y = v1.z*v2.x - v1.x*v2.z;
		crossP.z = v1.x*v2.y - v1.y*v2.x;

		len = sqrt(crossP.x*crossP.x + crossP.y*crossP.y + crossP.z*crossP.z);

		crossP.x = -crossP.x / len;
		crossP.y = -crossP.y / len;
		crossP.z = -crossP.z / len;

		normList[faceList[i].v1].x = normList[faceList[i].v1].x + crossP.x;
		normList[faceList[i].v1].y = normList[faceList[i].v1].y + crossP.y;
		normList[faceList[i].v1].z = normList[faceList[i].v1].z + crossP.z;
		normList[faceList[i].v2].x = normList[faceList[i].v2].x + crossP.x;
		normList[faceList[i].v2].y = normList[faceList[i].v2].y + crossP.y;
		normList[faceList[i].v2].z = normList[faceList[i].v2].z + crossP.z;
		normList[faceList[i].v3].x = normList[faceList[i].v3].x + crossP.x;
		normList[faceList[i].v3].y = normList[faceList[i].v3].y + crossP.y;
		normList[faceList[i].v3].z = normList[faceList[i].v3].z + crossP.z;
		normCount[faceList[i].v1]++;
		normCount[faceList[i].v2]++;
		normCount[faceList[i].v3]++;
	}
	for (i = 0; i < verts; i++) {
		normList[i].x = (float)sign*normList[i].x / (float)normCount[i];
		normList[i].y = (float)sign*normList[i].y / (float)normCount[i];
		normList[i].z = (float)sign*normList[i].z / (float)normCount[i];
	}
}


// The display function. It is called whenever the window needs
// redrawing (ie: overlapping window moves, resize, maximize)
// You should redraw your polygons here
void display(void) {
	// Clear the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (PERSPECTIVE) {
		// Perscpective Projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)window_width / window_height, 0.01, 10000);
		glutSetWindowTitle("Assignment 2 (perspective)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		cameraX = cosf(theta)*sinf(phi)*radius;
		cameraY = cosf(phi)*radius;
		cameraZ = sinf(theta)*sinf(phi)*radius;
	}
	else {
		// Orthogonal Projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-2.5, 2.5, -2.5, 2.5, -10000, 10000);
		glutSetWindowTitle("Assignment 2 (orthogonal)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		cameraX = cosf(alpha)*sinf(omega)*radius;
		cameraY = cosf(omega)*radius;
		cameraZ = sinf(alpha)*sinf(omega)*radius;
	}
	// Set the camera position, orientation and target
	gluLookAt(cameraX, cameraY, cameraZ, 0, 0, 0, 0, 1, 0);

	if (displayObj) {
		// Draw mesh object
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(1, 1, 1);
		for (int f = 0; f < faces; f++) {
			glBegin(GL_TRIANGLES);
			glVertex3f(vertList[faceList[f].v1].x, vertList[faceList[f].v1].y, vertList[faceList[f].v1].z);
			glVertex3f(vertList[faceList[f].v2].x, vertList[faceList[f].v2].y, vertList[faceList[f].v2].z);
			glVertex3f(vertList[faceList[f].v3].x, vertList[faceList[f].v3].y, vertList[faceList[f].v3].z);
		}
		glEnd();
	}

	if (displayCoordAxes) {
		// Draw Coordinate Axes
		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 2, 0);
		glEnd();

		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(2, 0, 0);
		glEnd();

		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 2);
		glEnd();
	}

	// (Note that the origin is lower left corner)
	// (Note also that the window spans (0,1) )
	// Finish drawing, update the frame buffer, and swap buffers
	glutSwapBuffers();
}

// This function is called whenever the window is resized. 
// Parameters are the new dimentions of the window
void resize(int x, int y) {
	glViewport(0, 0, x, y);
	window_width = x;
	window_height = y;
	if (PERSPECTIVE) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)window_width / window_height, 0.01, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	printf("Resized to %d %d\n", x, y);
}

// This function is called whenever the mouse is pressed or released
// button is a number 0 to 2 designating the button
// state is 1 for release 0 for press event
// x and y are the location of the mouse (in window-relative coordinates)
void mouseButton(int button, int state, int x, int y) {
	printf("Mouse click at %d %d, button: %d, state: %d,\n", x, y, button, state);

	// Saves the x and y coordinates to a global variable when a mouse button is clicked
	clickX = x;
	clickY = y;

	// Checks if the left or right mouse button is clicked
	if (button == 0) {
		clickedButton = LEFT;
	}
	else if (button == 2) {
		clickedButton = RIGHT;
	}
}

// This function is called whenever the mouse is moved with a mouse button held down.
// x and y are the location of the mouse (in window-relative coordinates)
void mouseMotion(int x, int y) {
	printf("Mouse is at %d, %d\n", x, y);

	// Orbits around the origin when the left mouse button is clicked and dragged in either the x or y mouse axis.
	if (clickedButton == LEFT && PERSPECTIVE) {
		theta = (x - clickX) / SENSITIVITY + theta;
		phi = (y - clickY) / SENSITIVITY + phi;
		cameraX = cosf(theta)*sinf(phi)*radius;
		if (phi > (-PI + 0.1) && phi < -0.1) {
			prevPhi = phi;
		}
		else {
			phi = prevPhi;
		}
		cameraY = cosf(phi)*radius;
		cameraZ = sinf(theta)*sinf(phi)*radius;
		clickX = x;
		clickY = y;
	}
	else if (clickedButton == LEFT && !PERSPECTIVE) {
		alpha = (x - clickX) / SENSITIVITY + alpha;
		omega = (y - clickY) / SENSITIVITY + omega;
		cameraX = cosf(alpha)*sinf(omega)*radius;
		if (omega > (-PI + 0.1) && omega < -0.1) {
			prevOmega = omega;
		}
		else {
			omega = prevOmega;
		}
		cameraY = cosf(omega)*radius;
		cameraZ = sinf(alpha)*sinf(omega)*radius;
		clickX = x;
		clickY = y;
	}
	// Zooms in or out when the right mouse button is clicked and is moving along the y-mouse axis.
	else if (clickedButton == RIGHT && PERSPECTIVE) {
		radius = radius - ((y - clickY) / SENSITIVITY);
		if (radius > 1.0 && radius < 1000) {
			cameraX = cosf(theta)*sinf(phi)*radius;
			cameraY = cosf(phi)*radius;
			cameraZ = sinf(theta)*sinf(phi)*radius;
			clickY = y;
			prevRadius = radius;
		}
		else {
			radius = prevRadius;
		}
	}

	glutPostRedisplay();
}


// This function is called whenever there is a keyboard input
// key is the ASCII value of the key pressed
// x and y are the location of the mouse
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':
		// Quit
		exit(1);
		break;
	case 'p':
		// Toggle between perspective and orthogonal
		if (PERSPECTIVE) {
			// switch from perspective to orthogonal
			PERSPECTIVE = OFF;
		}
		else {
			// switch from orthogonal to perspective
			PERSPECTIVE = ON;
		}
		break;
	case 'a':
		// Toggle display of coordinate axes
		if (displayCoordAxes) {
			displayCoordAxes = OFF;
		}
		else {
			displayCoordAxes = ON;
		}
		break;
	case 's':
		// Toggle display of object
		if (displayObj) {
			displayObj = OFF;
		}
		else {
			displayObj = ON;
		}
		break;
	default:
		break;
	}

	// Schedule a new display event
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	// Create mesh from object file
	char *filename = "sphere.obj";
	meshReader(filename, 2);

	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Assignment 2 (orthogonal)");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboard);

	// Initialize GL
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2.5, 2.5, -2.5, 2.5, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);

	// Switch to main loop
	glutMainLoop();
	return 0;
}