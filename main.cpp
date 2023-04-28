#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "shader.h"
#include "shaderprogram.h"

#include <vector>
#include "Shape.h"
 
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

/*=================================================================================================
	OBJECTS
=================================================================================================*/

//VAO -> the object "as a whole", the collection of buffers that make up its data
//VBOs -> the individual buffers/arrays with data, for ex: one for coordinates, one for color, etc.

GLuint axis_VAO;
GLuint axis_VBO[2];

float axis_vertices[] = {
	//x axis
	-1.0f,  0.0f,  0.0f, 1.0f,
	1.0f,  0.0f,  0.0f, 1.0f,
	//y axis
	0.0f, -1.0f,  0.0f, 1.0f,
	0.0f,  1.0f,  0.0f, 1.0f,
	//z axis
	0.0f,  0.0f, -1.0f, 1.0f,
	0.0f,  0.0f,  1.0f, 1.0f
};

float axis_colors[] = {
	//x axis
	1.0f, 0.0f, 0.0f, 1.0f,//red
	1.0f, 0.0f, 0.0f, 1.0f,
	//y axis
	0.0f, 1.0f, 0.0f, 1.0f,//green
	0.0f, 1.0f, 0.0f, 1.0f,
	//z axis
	0.0f, 0.0f, 1.0f, 1.0f,//blue
	0.0f, 0.0f, 1.0f, 1.0f
};

//sphere
GLuint sphere_VAO;
GLuint sphere_VBO[3];

std::vector<glm::vec4> sphere_vertices;
std::vector<glm::vec4> sphere_colors;
std::vector<glm::vec4> sphere_normal;

GLuint floor_VAO;
GLuint floor_VBO[3];
std::vector<glm::vec4> floor_vertices;
std::vector<glm::vec4> floor_colors;
std::vector<glm::vec4> floor_normal;

GLuint cylinder_VAO;
GLuint cylinder_VBO[3];
std::vector<glm::vec4> cylinder_vertices;
std::vector<glm::vec4> cylinder_colors;
std::vector<glm::vec4> cylinder_normal; 

GLuint cone_VAO;
GLuint cone_VBO[3];
std::vector<glm::vec4> cone_vertices;
std::vector<glm::vec4> cone_colors;
std::vector<glm::vec4> cone_normal;

GLuint cuboid_VAO;
GLuint cuboid_VBO[3];
std::vector<glm::vec4> cuboid_vertices;
std::vector<glm::vec4> cuboid_colors;
std::vector<glm::vec4> cuboid_normal;

GLuint norm_VAO;
GLuint norm_VBO[2];
std::vector<glm::vec4> norm_vertices;
std::vector<glm::vec4> norm_colors;

//temp
GLuint triangle_VAO;
GLuint triangle_VBO[3]; 
std::vector<glm::vec4> triangle_vertices;
std::vector<glm::vec4> triangle_color;
std::vector<glm::vec4> triangle_normal;

//plane
GLuint plane_VAO;
GLuint plane_VBO[2];
std::vector<glm::vec4> plane_vertices;
std::vector<glm::vec4> plane_colors;


/*=================================================================================================
	HELPER FUNCTIONS
=================================================================================================*/

void window_to_scene( int wx, int wy, float& sx, float& sy )
{
	sx = ( 2.0f * (float)wx / WindowWidth ) - 1.0f;
	sy = 1.0f - ( 2.0f * (float)wy / WindowHeight );
}

/*=================================================================================================
	SHADERS
=================================================================================================*/
//eye position
struct EyePos {
	float x;
	float y;
	float z;
	EyePos() {
		x = 0.0f;
		y = 3.0f;
		z = 3.0f;
	}
};
EyePos eyePos;
void CreateTransformationMatrices( void )
{
	// PROJECTION MATRIX
	PerspProjectionMatrix = glm::perspective<float>( glm::radians( 60.0f ), (float)WindowWidth / (float)WindowHeight, 0.01f, 1000.0f );

	// VIEW MATRIX

	glm::vec3 eye   ( eyePos.x, eyePos.y, eyePos.z );
	glm::vec3 center( 0.0f, 0.0f, 0.0f );
	glm::vec3 up    ( 0.0, 1.0, 0.0 );

	PerspViewMatrix = glm::lookAt( eye, center, up );

	// MODEL MATRIX
	PerspModelMatrix = glm::mat4( 1.0 );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationX ), glm::vec3( 1.0, 0.0, 0.0 ) );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationY ), glm::vec3( 0.0, 1.0, 0.0 ) );
	PerspModelMatrix = glm::scale( PerspModelMatrix, glm::vec3( perspZoom ) );
}

//RAY TRACING
//struct ViewP {
//	float x;
//	float y;
//	float z;
//	glm::vec3 v1;
//	glm::vec3 v2;
//
//	ViewP() {
//		//camera vector
//		glm::vec3 v0(eyePos.x, eyePos.y, eyePos.z);
//		std::cout << v0.x << " " << v0.y << " " << v0.z << std::endl;
//
//		//perpenducular vector to the direction vector --> x direction
//		v1 = glm::vec3(-1.0f, 0.0f, 0.0f);
//		std::cout << v1.x << " " << v1.y << " " << v1.z << std::endl;
//
//
//		//cross product of direction vector and perpendicular vector 
//		v2 = glm::vec3(glm::normalize(cross(v1, v0)));
//		std::cout << v2.x << " " << v2.y << " " << v2.z << std::endl;
//		
//		//distance of camera and plane
//		x = eyePos.x -1.0f + v1.x;
//		y = eyePos.y -1.0f - v1.y - v2.y;
//		z = eyePos.z -1.0f - v1.z - v2.z;
//		std::cout << x << " " << y << " " << z << std::endl;
//
//
//	}
//
//};

//scalar multiply vec3
glm::vec3 operator*(float scalar, const glm::vec3& v) {
	return glm::vec3(scalar * v.x, scalar * v.y, scalar * v.z);
}

glm::vec3 RayTrace(glm::vec3 s, glm::vec3 u, int maxDepth) {
	//s is the starting postion of the ray
	//u is the unit vector in the direction of the ray
	//depth is the trace depth
	//return value is a 3 tuple of color vlaue(R,G,B);

	// Part 1:: nonrecursive computations

	float intersection = 10.0f;
	glm::vec3 color(0.0f,0.0f,0.0f);
	glm::vec3 z(0.0f,0.0f,0.0f);

	int depth = maxDepth;
	float p_rg = 1.0f;
	float p_tg = 1.0f;
	for (float t = 1.0f; t <= 10.0f; t+= 0.01f) {
		//r is a vector that goes from the center to the unit vector starting at the camera at t
		//r equation : s = starting point of ray at camera
		//				 u = direction vector of the ray
		//				 t = parameter
		glm::vec3 r(glm::normalize(s + (t * u)));
		//normal vector of triangle
		glm::vec3 v0(triangle_vertices[0]);
		glm::vec3 v1(triangle_vertices[1]);
		glm::vec3 v2(triangle_vertices[2]);

		glm::vec3 n(glm::normalize(glm::cross(v1 - v0, v2 - v0)));

		//dot product of the triangle's normal vector with the ray's direction vector
		float d = glm::dot(n, r);

		//std::cout << d << std::endl;

		//if r dot n is zero it means that the two are orthoganal
		//if r and n are orthoganal that means at t the ray intersect with the shape
		
		//if intersected
		//let z = first intersection point
		//let n = normal at the intersection point
		if (d < 0.001f && d > -0.001f ) {
			//std::cout << d << std::endl;
			return glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		}


	}
	//if not intersected
	return color;
}

void RayTraceMain() {
	glm::vec4 x(eyePos.x, eyePos.y, eyePos.z,1.0f);  // let x be the postion of the viewer
	int maxDepth = 3;							// let maxDepth be a positive integer

	float length = 2.0f / 800.0f; //length of the pixel
	//For each pixel p in the viewport
	for (int i = 0; i < plane_vertices.size();i += 6) {
		//calculate the middle of the pixel p 
		glm::vec4 v0(plane_vertices[i]);
		//find the middle of the width and height of the pixel by dividing two
		float midpoint = length / 2.0f;
		glm::vec4 p(v0 + glm::vec4(midpoint)); // found p

		//set u = unit vector in the direction from x to p (camera to pixel);
		glm::vec4 u(glm::normalize(p - x));
		//call RayTrace
		glm::vec3 color(RayTrace(x, u, maxDepth));
		//assign pixel p the color return by Ray Trace
		for (int j = 0; j < 6;j++) {
			plane_colors.push_back(glm::vec4(color,1.0f));
		}
	}
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
	glGenVertexArrays( 1, &axis_VAO ); //generate 1 new VAO, its ID is returned in axis_VAO
	glBindVertexArray( axis_VAO ); //bind the VAO so the subsequent commands modify it

	glGenBuffers( 2, &axis_VBO[0] ); //generate 2 buffers for data, their IDs are returned to the axis_VBO array

	// first buffer: vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[0] ); //bind the first buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_vertices ), axis_vertices, GL_STATIC_DRAW ); //send coordinate array to the GPU
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 0, made up of 4 floats
	glEnableVertexAttribArray( 0 );

	// second buffer: colors
	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[1] ); //bind the second buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_colors ), axis_colors, GL_STATIC_DRAW ); //send color array to the GPU
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 1, made up of 4 floats
	glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 ); //unbind when done

	//NOTE: You will probably not use an array for your own objects, as you will need to be
	//      able to dynamically resize the number of vertices. Remember that the sizeof()
	//      operator will not give an accurate answer on an entire vector. Instead, you will
	//      have to do a calculation such as sizeof(v[0]) * v.size().
}

//
//void CreateMyOwnObject( void ) ...
//
void CreateTriangle() {

	glm::vec4 v0(-0.5f, 0.0f, 0.0f, 1.0f);
	glm::vec4 v1(0.5f, 0.0f, 0.0f, 1.0f);
	glm::vec4 v2(0.0f, 0.5f, 0.0f, 1.0f);
	triangle_vertices.push_back(v0);
	triangle_vertices.push_back(v1);
	triangle_vertices.push_back(v2);

	for (int i = 0; i < triangle_vertices.size(); i++) {
		triangle_color.push_back(glm::vec4(1.0f,0.0f,0.0f,1.0f));
	}

	for (int i = 0; i < triangle_vertices.size(); i++) {
		triangle_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)), 1.0f));
	}

	glGenVertexArrays(1, &triangle_VAO);
	glBindVertexArray(triangle_VAO);

	glGenBuffers(3, &triangle_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, triangle_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, triangle_vertices.size() * sizeof(glm::vec4), &triangle_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, triangle_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, triangle_color.size() * sizeof(glm::vec4), &triangle_color[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, triangle_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, triangle_normal.size() * sizeof(glm::vec4), &triangle_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void CreatePlane(void) {
	glm::vec3 cameraPos(0.0f, 3.0f, 3.0f);
	//vector of the plane
	glm::vec3 normalv1(1.0f, 0.0f, 0.0f);
	glm::vec3 normalv2(glm::normalize(glm::cross(cameraPos, normalv1)));

	//get center plane 
	glm::vec3 planePos(0.0f, 2.0f, 2.0f);
	//get bottom left starting point 
	planePos = planePos - glm::vec3(normalv1.x, normalv2.y, normalv2.z);

	//calculate the width and height difference with 800 pixels in 2.0f x 2.0f 
	float iDif = 2.0f / 800.0f;
	float jDif = 2.0f / 800.0f;

	float initPlanex = planePos.x;
	//calculate the width and heihght of y and z with idif being the hypotnuse using the equation y^2 + z^2 = idif^2 
	float yzDif = sqrt(jDif * jDif/2);
	for (float i = -1.0f; i < 1.0f;i += iDif) { // in the y and z direction
		for (float j = -1.0f; j < 1.0f; j += jDif) { // in the x direction
			//jDif is the hypotnuse, find the y and z values with the hypotnuse
			//first triangle
			plane_vertices.push_back(glm::vec4(planePos.x, planePos.y, planePos.z, 1.0f));
			plane_vertices.push_back(glm::vec4(planePos.x + iDif, planePos.y, planePos.z, 1.0f));
			plane_vertices.push_back(glm::vec4(planePos.x, planePos.y + yzDif , planePos.z - yzDif, 1.0f)); 
			//second triangle
			plane_vertices.push_back(glm::vec4(planePos.x, planePos.y + yzDif, planePos.z - yzDif, 1.0f)); 
			plane_vertices.push_back(glm::vec4(planePos.x + iDif, planePos.y, planePos.z, 1.0f));
			plane_vertices.push_back(glm::vec4(planePos.x + iDif, planePos.y + yzDif, planePos.z - yzDif, 1.0f));
			planePos.x += iDif;
		}
		planePos.x = initPlanex;
		planePos.y += yzDif;
		planePos.z -= yzDif;
	}
	RayTraceMain();

	glGenVertexArrays(1, &plane_VAO);
	glBindVertexArray(plane_VAO);

	glGenBuffers(2, &plane_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, plane_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, plane_vertices.size() * sizeof(glm::vec4), &plane_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, plane_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, plane_colors.size() * sizeof(glm::vec4), &plane_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	////thrid array
	//glBindBuffer(GL_ARRAY_BUFFER, plane_VBO[2]);
	//glBufferData(GL_ARRAY_BUFFER, triangle_normal.size() * sizeof(glm::vec4), &triangle_normal[0], GL_STATIC_DRAW);
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	//glEnableVertexAttribArray(2);

	glBindVertexArray(0);
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
		//case 'w':
		//{
		//	eyePos.z -= 0.05f;
		//	break;
		//}
		//case 's':
		//{
		//	eyePos.z += 0.05f;
		//	break;
		//}
		//case 'd':
		//{
		//	sphere_vertices.clear();
		//	sphere_colors.clear();
		//	sphere_normal.clear();
		//	spherex += 0.01f; spherez += 0.01f;
		//	CreateSphere(spherex, spherey, spherez, r);

		//	break;
		//}
		case 'f':
		{
			glDeleteVertexArrays(1, &cylinder_VAO);
			std::cout << "removal" << std::endl;
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

	// Update transformation matrices
	CreateTransformationMatrices();

	// Choose which shader to user, and send the transformation matrix information to it
	PerspectiveShader.Use();
	PerspectiveShader.SetUniform( "projectionMatrix", glm::value_ptr( PerspProjectionMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "viewMatrix", glm::value_ptr( PerspViewMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "modelMatrix", glm::value_ptr( PerspModelMatrix ), 4, GL_FALSE, 1 );

	// Drawing in wireframe?
	if( draw_wireframe == true )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// Bind the axis Vertex Array Object created earlier, and draw it
	glBindVertexArray( axis_VAO );
	glDrawArrays( GL_LINES, 0, 6 ); // 6 = number of vertices in the object

	//
	// Bind and draw your object here
	//
	//sphere
	glBindVertexArray(sphere_VAO);
	glDrawArrays(GL_TRIANGLES, 0, sphere_vertices.size());
	//cylinder
	glBindVertexArray(cylinder_VAO);
	glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
	//cone
	glBindVertexArray(cone_VAO);
	glDrawArrays(GL_TRIANGLES, 0, cone_vertices.size());
	//rectangular prism
	glBindVertexArray(cuboid_VAO);
	glDrawArrays(GL_TRIANGLES, 0, cuboid_vertices.size());
	//normal
	glBindVertexArray(norm_VAO);
	glDrawArrays(GL_LINES, 0, norm_vertices.size());

	//floor
	glBindVertexArray(floor_VAO);
	glDrawArrays(GL_TRIANGLES, 0, floor_vertices.size());
	// Unbind when done

	//triangle
	glBindVertexArray(triangle_VAO);
	glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());

	//plane
	glBindVertexArray(plane_VAO);
	glDrawArrays(GL_TRIANGLES, 0, plane_vertices.size());

	glBindVertexArray(0);


	// Swap the front and back buffers
	glutSwapBuffers();
}
float spherex = 0.0f, spherey = 0.5f, spherez = 0.0f, r = 0.5f;
float theta = 0;
void update(int value) {
	sphere_vertices.clear();
	sphere_colors.clear();
	sphere_normal.clear();
	theta += 1;
	spherex += cos(theta); spherez += sin(theta);
	CreateSphere(spherex, spherey, spherez, r);
	glutPostRedisplay();
	glutTimerFunc(60, update, 0);
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

	//
	// Consider calling a function to create your object here
	//
	//CreateSphere(0.0f,0.7f,0.0f,0.7f);
	CreateSphere(1.0f, 0.5f, 1.0f,0.5f); //(x,y,z,radius)
	CreateSphere(3.0f, 0.0f, 3.0f, 0.1f);
	CreateCylinder(-1.0f, 0.0f, 1.0f,0.5f,1.0f);// (x,y,z,radius, height)
	CreateCone(0.0f, 0.0f, -1.5f, 0.5f, 1.0f);// (x,y,z,radius, height)
	CreateCuboid(-1.5f, 0.0f, -1.5f, 1.0f, 1.5f, 1.25f);//(x,y,z,width,length,height)
	CreateCylinder(2.0f, 0.0f, 0.0f, 0.2f, 2.0f);
	
	CreateTriangle();
	CreatePlane();
	//CreateNormLines();


	CreateFloor();

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
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

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

	//moving function
	//glutTimerFunc(25, update, 0);
	// Do program initialization
	init();

	// Enter the main loop
	glutMainLoop();

	return EXIT_SUCCESS;
}
