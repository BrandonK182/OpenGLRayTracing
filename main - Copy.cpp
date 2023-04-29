#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "shader.h"
#include "shaderprogram.h"

#include <vector>
 
/*=================================================================================================
	DOMAIN
=================================================================================================*/

// Window dimensions
const int InitWindowWidth  = 800;
const int InitWindowHeight = 800;
int WindowWidth  = InitWindowWidth;
int WindowHeight = InitWindowHeight;

// Last mouse cursor position
int LastMousePosX = 0;
int LastMousePosY = 0;

// Arrays that track which keys are currently pressed
bool key_states[256];
bool key_special_states[256];
bool mouse_states[8];

// Other parameters
bool draw_wireframe = false;
	//eye position
struct EyePos {
	float x;
	float y;
	float z;
	EyePos() {
		x = 0.0f;
		y = 0.0f;
		z = 3.0f;
	}
};

float maxDistance = 100;

/*=================================================================================================
	SHADERS & TRANSFORMATIONS
=================================================================================================*/

ShaderProgram PassthroughShader;
ShaderProgram PerspectiveShader;

glm::mat4 PerspProjectionMatrix( 1.0f );
glm::mat4 PerspViewMatrix( 1.0f );
glm::mat4 PerspModelMatrix( 1.0f );

float perspZoom = 1.0f, perspSensitivity = 0.35f;
float perspRotationX = 0.0f, perspRotationY = 0.0f;

EyePos eyePos;
glm::vec3 eyePosition(eyePos.x, eyePos.y, eyePos.z);

/*=================================================================================================
	OBJECTS
=================================================================================================*/

//VAO -> the object "as a whole", the collection of buffers that make up its data
//VBOs -> the individual buffers/arrays with data, for ex: one for coordinates, one for color, etc.

GLuint VAO;
GLuint VBO[2];

//grid of whole window containing R,G,B values
glm::vec3 viewPlaneColor[InitWindowWidth][InitWindowHeight];
glm::vec3 viewPlaneCoor[InitWindowWidth][InitWindowHeight];
glm::vec3 viewPlaneVect[InitWindowWidth][InitWindowHeight];
std::vector<float> objectVert;
std::vector<float> objectColor;
std::vector<glm::vec3> normals;
std::vector<glm::vec3> midpoints;

glm::vec3 lightSource;

/*=================================================================================================
	HELPER FUNCTIONS
=================================================================================================*/

void window_to_scene( int wx, int wy, float& sx, float& sy )
{
	sx = ( 2.0f * (float)wx / WindowWidth ) - 1.0f;
	sy = 1.0f - ( 2.0f * (float)wy / WindowHeight );
}

void updateViewVector()
{
	float res = 800;
	eyePosition = glm::vec3(eyePos.x, eyePos.y, eyePos.z);
	for (int i = 0; i < res; i++)
	{
		for (int j = 0; j < res; j++)
		{
			//vector from the camera to the pixel of the view plane
			viewPlaneVect[i][j] = viewPlaneCoor[i][j] - eyePosition;
		}
	}
}

void moveCamera(float x, float y, float z)
{
	int res = 800;
	eyePos.x += x;
	eyePos.y += y;
	eyePos.z += z;
	for (int i = 0; i < res; i++)
	{
		for (int j = 0; j < res; j++)
		{
			viewPlaneCoor[i][j][0] += x;
			viewPlaneCoor[i][j][1] += y;
			viewPlaneCoor[i][j][2] += z;
		}	
	}
}

bool SameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b)
{
	glm::vec3 cp1 = glm::cross(b - a, p1 - a);
	glm::vec3 cp2 = glm::cross(b - a, p2 - a);
	return glm::dot(cp1, cp2) >= 0;
}

bool PointInTriangle(glm::vec3 p, glm::vec3 vert1, glm::vec3 vert2, glm::vec3 vert3)
{
	return SameSide(p, vert1, vert2, vert3) && SameSide(p, vert2, vert1, vert3) && SameSide(p, vert3, vert1, vert2);
}

bool checkLight(glm::vec3 point, glm::vec3 light, glm::vec3 normal)
{
	glm::vec3 vector = light - point;

	//check if the light is the opposite direction to the triangle face
	if (glm::dot(vector, normal) < 0)
		return false;
	glm::normalize(vector);
	float lowestT = maxDistance;
	for (int i = 0; i < normals.size(); i++)
	{
		//std::cout << "comparing to polygon " << i << std::endl;
		float t = (glm::dot(midpoints[i], normals[i]) - glm::dot(point, normals[i]) / (glm::dot(vector, normals[i])));
		glm::vec3 ray = point + (t * vector);

		//std::cout << "creating the vectors for each of the object" << std::endl;
		glm::vec3 v1(objectVert[i * 12], objectVert[i * 12 + 1], objectVert[i * 12 + 2]);
		glm::vec3 v2(objectVert[i * 12 + 4], objectVert[i * 12 + 5], objectVert[i * 12 + 6]);
		glm::vec3 v3(objectVert[i * 12 + 8], objectVert[i * 12 + 9], objectVert[i * 12 + 10]);

		//std::cout << "checking if the point is in the triangle" << std::endl;
		if (PointInTriangle(ray, v1, v2, v3))
		{
			//std::cout << " CONNECTION!!!!!!!       point connects to triangle" << std::endl;
			//marks the ray as touching the triangle at some point
			//checks if the contact is the closest contact point
			if (t < lowestT && t > 0.01f)
			{
				//std::cout << "lower than max" << std::endl;
				return false;
			}
		}
	}
	//does not intersect with any other triangles
	return true;
}

glm::vec3 RayTrace(glm::vec3 s, glm::vec3 u, int depth) {
	//ray in point s in direction u
	bool intersect = false;
	float lowestT = maxDistance;
	int lowestPos = 0;
	glm::vec3 ray; 
	//std::cout << "begin raytracing     S: " << s[0] <<" , " <<  s[1]  << ", " << s[2] << " u: " << u[0] <<", " <<  u[1] << ", " << u[2] << std::endl;
	for (int i = 0; i < normals.size(); i++)
	{
		//std::cout << "comparing to polygon " << i << std::endl;
		float t = (glm::dot(midpoints[i], normals[i]) - glm::dot(s, normals[i]) / (glm::dot(u, normals[i])));
		ray = s + (t * u);

		//std::cout << "creating the vectors for each of the object" << std::endl;
		glm::vec3 v1(objectVert[i * 12], objectVert[i * 12 + 1], objectVert[i * 12 + 2]);
		glm::vec3 v2(objectVert[i * 12 + 4], objectVert[i * 12 + 5], objectVert[i * 12 + 6]);
		glm::vec3 v3(objectVert[i * 12 + 8], objectVert[i * 12 + 9], objectVert[i * 12 + 10]);

		//std::cout << "checking if the point is in the triangle" << std::endl;
		if (PointInTriangle(ray, v1, v2, v3))
		{
			//std::cout << " CONNECTION!!!!!!!       point connects to triangle" << std::endl;
			//marks the ray as touching the triangle at some point
			//checks if the contact is the closest contact point
			if (t < lowestT && t > 0.01f)
			{
				//std::cout << "lower than max" << std::endl;
				intersect = true;
				lowestT = t;
				lowestPos = i;
			}		
		}
	}	

	//std::cout << "beginning check if item intersects" << std::endl;
	//send white value if empty
	if (!intersect)
		return glm::vec3(1.0f, 1.0f, 1.0f);


	//ray = s + (lowestT * u);
	//std::cout << "lowest ray intersection " << ray[0] << "," << ray[1] << "," << ray[2] << std::endl;
	ray = s + (lowestT * u);
	glm::vec3 pixColor = glm::vec3(objectColor[lowestPos * 12], objectColor[lowestPos * 12 + 1], objectColor[lowestPos * 12 + 2]);
	if(checkLight(ray, lightSource, normals[lowestPos]))
		return pixColor;
	return pixColor*0.75f;
	//return the color of of of the verticies on the triangle with the lowest distance
	//change later
	//return glm::vec3(objectColor[lowestPos*12], objectColor[lowestPos * 12], objectColor[lowestPos * 12]);
}

void RayTraceMain()
{
	for (int i = 0; i < WindowWidth; i++)
	{
		for (int j = 0; j < WindowHeight; j++)
		{
			//std::cout<<"drawing pixel " << i << ", " << j << std::endl;
			glm::vec3 u = glm::normalize(viewPlaneVect[i][j]);
			//std::cout << "normalized direction u" << std::endl;
			glm::vec3 color = RayTrace(eyePosition, u, 0);
			//std::cout << "finish ray trace" << std::endl;
			
			glBegin(GL_POINTS);
				glColor3f(color[0],color[1],color[2]);
				glVertex2i(i, j);
			glEnd();
		}
	}
	std::cout << "draw cycle complete" << std::endl;
	updateViewVector();
}

//creates plane that exists in -1x to 1x and -1y to 1y
void CreateViewPlane()
{
	float max = 2.0f;
	float res = 800;
	float pixelSize = max / res;
	for (int i = 0; i < res; i++)
	{
		float x = (i * pixelSize) - (max / 2);
		for (int j = 0; j < res; j++)
		{
			float y = (j * pixelSize) - (max / 2);
			viewPlaneCoor[i][j][0] = x + (pixelSize / 2);
			viewPlaneCoor[i][j][1] = y + (pixelSize / 2);
			viewPlaneCoor[i][j][2] = 1.5f;
		}
	}

	updateViewVector();
}

void BuildTestPyramid()
{
	//back face
	objectVert.push_back(0.0f);
	objectVert.push_back(0.5f);
	objectVert.push_back(-1.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(-0.5f);
	objectVert.push_back(-1.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(-0.5f);
	objectVert.push_back(1.0f);

	/*
	//left face
	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	//floor face
	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(-1.0f);
	objectVert.push_back(1.0f);

	//slanted face
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(1.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(1.0f);

	objectVert.push_back(0.0f);
	objectVert.push_back(0.0f);
	objectVert.push_back(-1.0f);
	objectVert.push_back(1.0f);

	*/
	
	for (int i = 0; i < objectVert.size()/4; i++)
	{
		objectColor.push_back(1.0f);
		objectColor.push_back(0.0f);
		objectColor.push_back(0.0f);
		objectColor.push_back(1.0f);
	}
}

void createNormals()
{
	for (int i = 0; i < (objectVert.size() / 12); i++)
	{
		glm::vec3 v1(objectVert[i*12], objectVert[i * 12+1], objectVert[i * 12+2]);
		glm::vec3 v2(objectVert[i * 12 + 4], objectVert[i * 12 + 5], objectVert[i * 12 + 6]);
		glm::vec3 v3(objectVert[i * 12 + 8], objectVert[i * 12 + 9], objectVert[i * 12 + 10]);

		glm::vec3 AC = v3 - v1;
		glm::vec3 CB = v3 - v2;
		glm::vec3 direction = glm::cross(AC, CB);
		glm::normalize(direction);
		normals.push_back(direction);

		glm::vec3 center = { (v1[0] + v2[0] + v3[0]) / 3,
				(v1[1] + v2[1] + v3[1]) / 3,
				(v1[2] + v2[2] + v3[2]) / 3 };
		midpoints.push_back(center);
	}
}

/*=================================================================================================
	SHADERS
=================================================================================================*/

void CreateTransformationMatrices( void )
{
	// PROJECTION MATRIX
	PerspProjectionMatrix = glm::perspective<float>( glm::radians( 60.0f ), (float)WindowWidth / (float)WindowHeight, 0.01f, 1000.0f );

	// VIEW MATRIX

	glm::vec3 eye   ( eyePos.x, eyePos.y, eyePos.z );
	glm::vec3 center( 0.0, 0.0, 0.0 );
	glm::vec3 up    ( 0.0, 1.0, 0.0 );

	PerspViewMatrix = glm::lookAt( eye, center, up );

	// MODEL MATRIX
	PerspModelMatrix = glm::mat4( 1.0 );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationX ), glm::vec3( 1.0, 0.0, 0.0 ) );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationY ), glm::vec3( 0.0, 1.0, 0.0 ) );
	PerspModelMatrix = glm::scale( PerspModelMatrix, glm::vec3( perspZoom ) );
}

void CreateShaders( void )
{
	// Renders without any transformations
	PassthroughShader.Create( "./shaders/simple.vert", "./shaders/simple.frag" );

	// Renders using perspective projection
	PerspectiveShader.Create( "./shaders/persp.vert", "./shaders/persp.frag" );

	//
	// Additional shaders would be defined here
	//
}

/*=================================================================================================
	BUFFERS
=================================================================================================*/

void CreateAxisBuffers( void )
{
	glGenVertexArrays( 1, &VAO ); //generate 1 new VAO, its ID is returned in axis_VAO
	glBindVertexArray( VAO ); //bind the VAO so the subsequent commands modify it

	glGenBuffers( 2, &VBO[0] ); //generate 2 buffers for data, their IDs are returned to the axis_VBO array

	/*
	// first buffer: vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, VBO[0] ); //bind the first buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW ); //send coordinate array to the GPU
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 0, made up of 4 floats
	glEnableVertexAttribArray( 0 );

	// second buffer: colors
	glBindBuffer( GL_ARRAY_BUFFER, VBO[1] ); //bind the second buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( colors ), colors, GL_STATIC_DRAW ); //send color array to the GPU
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 1, made up of 4 floats
	glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 ); //unbind when done

	//NOTE: You will probably not use an array for your own objects, as you will need to be
	//      able to dynamically resize the number of vertices. Remember that the sizeof()
	//      operator will not give an accurate answer on an entire vector. Instead, you will
	//      have to do a calculation such as sizeof(v[0]) * v.size().
	*/
}

/*=================================================================================================
	CALLBACKS
=================================================================================================*/

//-----------------------------------------------------------------------------
// CALLBACK DOCUMENTATION
// https://www.opengl.org/resources/libraries/glut/spec3/node45.html
// http://freeglut.sourceforge.net/docs/api.php#WindowCallback
//-----------------------------------------------------------------------------

void idle_func()
{
	//uncomment below to repeatedly draw new frames
	glutPostRedisplay();
}

void reshape_func( int width, int height )
{
	WindowWidth  = width;
	WindowHeight = height;

	glViewport( 0, 0, width, height );
	glutPostRedisplay();
}

void keyboard_func( unsigned char key, int x, int y )
{
	key_states[ key ] = true;

	switch( key )
	{
		case 'w':
		{
			moveCamera(0.0, 0.0, -0.05f);
			break;
		}
		case 's':
		{
			moveCamera(0.0, 0.0, 0.05f);
			break;
		}
		case 'a':
		{
			moveCamera(-0.05f, 0.0, 0.0);
			break;
		}
		case 'd':
		{
			moveCamera(0.05f, 0.0, 0.0);
			break;
		}
		case '1':
		{
			draw_wireframe = !draw_wireframe;
			if( draw_wireframe == true )
				std::cout << "Wireframes on.\n";
			else
				std::cout << "Wireframes off.\n";
			break;
		}

		// Exit on escape key press
		case '\x1B':
		{
			exit( EXIT_SUCCESS );
			break;
		}
	}
}

void key_released( unsigned char key, int x, int y )
{
	key_states[ key ] = false;
}

void key_special_pressed( int key, int x, int y )
{
	key_special_states[ key ] = true;
}

void key_special_released( int key, int x, int y )
{
	key_special_states[ key ] = false;
}

void mouse_func( int button, int state, int x, int y )
{
	// Key 0: left button
	// Key 1: middle button
	// Key 2: right button
	// Key 3: scroll up
	// Key 4: scroll down

	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( button == 3 )
	{
		perspZoom += 0.03f;
	}
	else if( button == 4 )
	{
		if( perspZoom - 0.03f > 0.0f )
			perspZoom -= 0.03f;
	}

	mouse_states[ button ] = ( state == GLUT_DOWN );

	LastMousePosX = x;
	LastMousePosY = y;
}

void passive_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	LastMousePosX = x;
	LastMousePosY = y;
}

void active_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( mouse_states[0] == true )
	{
		perspRotationY += ( x - LastMousePosX ) * perspSensitivity;
		perspRotationX += ( y - LastMousePosY ) * perspSensitivity;
	}

	LastMousePosX = x;
	LastMousePosY = y;
}

/*=================================================================================================
	RENDERING
=================================================================================================*/

void display_func( void )
{
	// Clear the contents of the back buffer
	
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D( 0.0, 800.0, 800.0, 0.0);
	
	
	RayTraceMain();
	
	/*
	for (int i = 0; i < WindowWidth; i++)
	{
		for (int j = 0; j < WindowHeight; j++)
		{
			//for loop to paint each pixel half way
			for (int k = 0; k < 3; k++)
			{
				viewPlaneColor[i][j][k] = 1.0f;
			}
			glBegin(GL_POINTS);
				glColor3f(viewPlaneColor[i][j][0], viewPlaneColor[i][j][1], viewPlaneColor[i][j][2]);
				glVertex2i(i, j);
			glEnd();
		}
	}
	*/

	// Swap the front and back buffers
	glutSwapBuffers();
	
}

/*=================================================================================================
	INIT
=================================================================================================*/

void init( void )
{
	// Print some info
	std::cout << "Vendor:         " << glGetString( GL_VENDOR   ) << "\n";
	std::cout << "Renderer:       " << glGetString( GL_RENDERER ) << "\n";
	std::cout << "OpenGL Version: " << glGetString( GL_VERSION  ) << "\n";
	std::cout << "GLSL Version:   " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n\n";

	// Set OpenGL settings
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); // background color
	glEnable( GL_DEPTH_TEST ); // enable depth test
	glEnable( GL_CULL_FACE ); // enable back-face culling

	// Create shaders
	CreateShaders();

	// Create axis buffers
	CreateAxisBuffers();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 800.0, 800.0, 0.0);

	//
	// Consider calling a function to create your object here
	//
	std::cout << "starting view plane" << std::endl;
	CreateViewPlane();
	std::cout << "building test pyramid" << std::endl;
	BuildTestPyramid();
	std::cout << "finished test pyramid" << std::endl;
	createNormals();
	
	lightSource = glm::vec3(0.0, 3.0, -2.0);

	std::cout << "Finished initializing...\n\n";
}

/*=================================================================================================
	MAIN
=================================================================================================*/

int main( int argc, char** argv )
{
	// Create and initialize the OpenGL context
	glutInit( &argc, argv );
	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( InitWindowWidth, InitWindowHeight );
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );

	glutCreateWindow( "CSE-170 Computer Graphics" );

	// Initialize GLEW
	GLenum ret = glewInit();
	if( ret != GLEW_OK ) {
		std::cerr << "GLEW initialization error." << std::endl;
		glewGetErrorString( ret );
		return -1;
	}

	// Register callback functions
	glutDisplayFunc( display_func );
	glutIdleFunc( idle_func );
	glutReshapeFunc( reshape_func );
	glutKeyboardFunc( keyboard_func );
	glutKeyboardUpFunc( key_released );
	glutSpecialFunc( key_special_pressed );
	glutSpecialUpFunc( key_special_released );
	glutMouseFunc( mouse_func );
	glutMotionFunc( active_motion_func );
	glutPassiveMotionFunc( passive_motion_func );

	// Do program initialization
	init();

	// Enter the main loop
	glutMainLoop();

	return EXIT_SUCCESS;
}
