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
EyePos eyePos;
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

//(x,y,z) = (rcos(theta)sin(phi) , rsin(theta)sin(phi), rcos(phi) )
//float pi = 3.14159;
//
//float n = 20.0f;
//void CreateSphere(float x, float y , float z, float r) {
//	float thetaDif, phiDif;
//	thetaDif = phiDif = 2 * pi / n;
//
//	for (float phi = 0; phi < 2 * pi; phi += phiDif) {
//		for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
//			float Ax = r * cos(theta) * sin(phi);
//			float Ay = r * sin(theta) * sin(phi);
//			float Az = r * cos(phi);
//
//			float A_x = r * cos(theta) * sin(phi + phiDif);
//			float A_y = r * sin(theta) * sin(phi + phiDif);
//			float A_z = r * cos(phi + phiDif);
//
//			float Bx = r * cos(theta + thetaDif) * sin(phi);
//			float By = r * sin(theta + thetaDif) * sin(phi);
//			float Bz = r * cos(phi);
//
//			float B_x = r * cos(theta + thetaDif) * sin(phi + phiDif);
//			float B_y = r * sin(theta + thetaDif) * sin(phi + phiDif);
//			float B_z = r * cos(phi + phiDif);
//
//			glm::vec4 A(Ax + x, Ay + y, Az + z, 1.0f);
//			glm::vec4 A_(A_x + x, A_y + y, A_z + z, 1.0f);
//			glm::vec4 B(Bx + x, By + y, Bz + z, 1.0f); 
//			glm::vec4 B_(B_x + x, B_y + y, B_z + z, 1.0f);
//
//			sphere_vertices.push_back(A);
//			sphere_vertices.push_back(A_);
//			sphere_vertices.push_back(B);
//			sphere_vertices.push_back(B);
//			sphere_vertices.push_back(A_);
//			sphere_vertices.push_back(B_);
//
//			//normal
//			glm::vec3 directionA(Ax, Ay, Az);
//			glm::vec3 directionA_(A_x, A_y, A_z);
//			glm::vec3 directionB(Bx, By, Bz);
//			glm::vec3 directionB_(B_x, B_y, B_z);
//
//			glm::vec4 normalA(glm::normalize(directionA), 1.0f);
//			glm::vec4 normalA_(glm::normalize(directionA_), 1.0f);
//			glm::vec4 normalB(glm::normalize(directionB), 1.0f);
//			glm::vec4 normalB_(glm::normalize(directionB_), 1.0f);
//
//
//			sphere_normal.push_back(normalA);
//			sphere_normal.push_back(normalA_);
//			sphere_normal.push_back(normalB);
//			sphere_normal.push_back(normalB);
//			sphere_normal.push_back(normalA_);
//			sphere_normal.push_back(normalB_);
//		}
//	}
//
//
//	for (int i = 0; i < sphere_vertices.size(); i++) {
//		sphere_colors.push_back(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
//	}
//
//	glGenVertexArrays(1, &sphere_VAO);
//	glBindVertexArray(sphere_VAO);
//
//	glGenBuffers(3, &sphere_VBO[0]);
//	//first buffer
//	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(glm::vec4), &sphere_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(0);
//	//Second array
//	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, sphere_colors.size() * sizeof(glm::vec4), &sphere_colors[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(1);
//	//thrid array
//	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[2]);
//	glBufferData(GL_ARRAY_BUFFER, sphere_normal.size() * sizeof(glm::vec4), &sphere_normal[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glBindVertexArray(0);
//}
//
//
//void CreateCylinder(float x, float y, float z, float r, float height) {
//	float thetaDif = 2 * pi / n;
//	//top and bottom
//	for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
//		float Ax = r * cos(theta);
//		float Ay = 0.0f;
//		float Az = r* sin(theta);
//		float A_x = r * cos(theta+thetaDif);
//		float A_y = 0.0f;
//		float A_z = r * sin(theta+thetaDif);
//		//bottom
//		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + 0.01f + y, Az + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y +  0.01f + y, A_z + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(0.0f + x, 0.01f + y, 0.0f + z, 1.0f));
//		//top
//		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y + height, A_z + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + y + height, Az + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(0.0f + x, height + y, 0.0f + z, 1.0f));
//		//wall
//		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y, A_z + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + y, Az + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + height + y, Az + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + height + y, Az + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y + height, A_z + z, 1.0f));
//		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y, A_z + z, 1.0f));
//
//		//normal
//
//
//
//		glm::vec4 normalA(glm::normalize(glm::vec3(0.0f,-1.0f,0.0f)), 1.0f);
//		glm::vec4 normalB(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f);
//		glm::vec4 normalC(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f);
//
//		cylinder_normal.push_back(normalA);
//		cylinder_normal.push_back(normalB);
//		cylinder_normal.push_back(normalC);
//
//		normalA = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
//		normalB = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
//		normalC = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
//
//		cylinder_normal.push_back(normalA);
//		cylinder_normal.push_back(normalB);
//		cylinder_normal.push_back(normalC);
//
//
//
//		normalA = glm::vec4(glm::normalize(glm::vec3(A_x,A_y,A_z)), 1.0f);
//		normalB = glm::vec4(glm::normalize(glm::vec3(Ax, Ay, Az)), 1.0f);
//
//		
//		cylinder_normal.push_back(normalA);
//		cylinder_normal.push_back(normalB);
//		cylinder_normal.push_back(normalB);
//		cylinder_normal.push_back(normalB);
//		cylinder_normal.push_back(normalA);
//		cylinder_normal.push_back(normalA);
//
//
//
//	}
//	for (int i = 0;i < cylinder_vertices.size();i++) {
//		cylinder_colors.push_back(glm::vec4(0.6f, 1.0f, 0.6f, 1.0f));
//	}
//	glGenVertexArrays(1, &cylinder_VAO);
//	glBindVertexArray(cylinder_VAO);
//
//	glGenBuffers(3, &cylinder_VBO[0]);
//	//first buffer
//	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, cylinder_vertices.size() * sizeof(glm::vec4), &cylinder_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(0);
//	//Second array
//	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, cylinder_colors.size() * sizeof(glm::vec4), &cylinder_colors[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(1);
//	//thrid array
//	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[2]);
//	glBufferData(GL_ARRAY_BUFFER, cylinder_normal.size() * sizeof(glm::vec4), &cylinder_normal[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glBindVertexArray(0);
//}
//
//void CreateCone(float x, float y, float z, float r, float height) {
//	float thetaDif = 2 * pi / n;
//	for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
//		float Ax = r * cos(theta);
//		float Ay = 0.0f;
//		float Az = r * sin(theta);
//		float A_x = r * cos(theta + thetaDif);
//		float A_y = 0.0f;
//		float A_z = r * sin(theta + thetaDif);
//
//		glm::vec4 A(Ax + x, Ay + 0.01f + y, Az + z, 1.0f);
//		glm::vec4 A_(A_x + x, A_y + 0.01f + y, A_z + z, 1.0f);
//		//bottom
//		cone_vertices.push_back(A);
//		cone_vertices.push_back(A_);
//		cone_vertices.push_back(glm::vec4(0.0f + x, 0.01f + y, 0.0f + z, 1.0f));
//
//		glm::vec4 topCenter(0.0f + x, height + y, 0.0f + z, 1.0f);
//		//wall
//		cone_vertices.push_back(A_);
//		cone_vertices.push_back(A);
//		cone_vertices.push_back(topCenter);
//
//		//normal
//		//bottom
//		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
//		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
//		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
//
//		//wall
//		glm::vec3 directionA(Ax , Ay , Az );
//		glm::vec3 directionB(A_x , A_y, A_z );
//		glm::vec3 directionC(0.0f,height,0.0f);
//
//		cone_normal.push_back(glm::vec4(glm::normalize(directionB), 1.0f));
//		cone_normal.push_back(glm::vec4(glm::normalize(directionA), 1.0f));
//		cone_normal.push_back(glm::vec4(glm::normalize(directionC), 1.0f));
//
//
//	}
//
//	for (int i = 0; i < cone_vertices.size();i++) {
//		cone_colors.push_back(glm::vec4(1.0f, 0.6f, 0.2f, 1.0f));
//	}
//	glGenVertexArrays(1, &cone_VAO);
//	glBindVertexArray(cone_VAO);
//
//	glGenBuffers(3, &cone_VBO[0]);
//	//first buffer
//	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, cone_vertices.size() * sizeof(glm::vec4), &cone_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(0);
//	//Second array
//	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, cone_colors.size() * sizeof(glm::vec4), &cone_colors[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(1);
//	//thrid array
//	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[2]);
//	glBufferData(GL_ARRAY_BUFFER, cone_normal.size() * sizeof(glm::vec4), &cone_normal[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glBindVertexArray(0);
//}
//void CreateCuboid(float x,float y,float z,float width,float length,float height) {
//	glm::vec4 lowerLeft(-width / 2 + x, y + 0.0f, -length / 2 + z, 1.0f); 
//	glm::vec4 lowerRight(width / 2 + x, y + 0.0f, -length / 2 + z, 1.0f);
//	glm::vec4 topLeft(-width / 2 + x,y + 0.0f,  length / 2 + z, 1.0f);
//	glm::vec4 topRight(width / 2 + x,y + 0.0f,  length / 2 + z, 1.0f);
//	glm::vec4 lowerLeftH(-width / 2 + x, y + height, -length / 2 + z, 1.0f);
//	glm::vec4 lowerRightH(width / 2 + x, y + height, -length / 2 + z, 1.0f);
//	glm::vec4 topLeftH(-width / 2 + x, y + height, length / 2 + z, 1.0f);
//	glm::vec4 topRightH(width / 2 + x, y + height, length / 2 + z, 1.0f);
//
//	//floor
//	cuboid_vertices.push_back(lowerLeft);
//	cuboid_vertices.push_back(lowerRight);	
//	cuboid_vertices.push_back(topLeft);	
//	cuboid_vertices.push_back(topLeft);		
//	cuboid_vertices.push_back(lowerRight);	
//	cuboid_vertices.push_back(topRight);	
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4( glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
//	}
//	//ceiling
//	cuboid_vertices.push_back(lowerLeftH);
//	cuboid_vertices.push_back(topLeftH);
//	cuboid_vertices.push_back(lowerRightH);
//	cuboid_vertices.push_back(lowerRightH);
//	cuboid_vertices.push_back(topLeftH);
//	cuboid_vertices.push_back(topRightH);
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f));
//	}
//	//wall(1)
//	cuboid_vertices.push_back(lowerLeft);
//	cuboid_vertices.push_back(lowerLeftH);
//	cuboid_vertices.push_back(lowerRight);
//	cuboid_vertices.push_back(lowerRight);
//	cuboid_vertices.push_back(lowerLeftH);
//	cuboid_vertices.push_back(lowerRightH);
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)), 1.0f));
//	}
//	//wall(2)
//	cuboid_vertices.push_back(lowerRight);
//	cuboid_vertices.push_back(lowerRightH);
//	cuboid_vertices.push_back(topRight);
//	cuboid_vertices.push_back(topRight);
//	cuboid_vertices.push_back(lowerRightH);
//	cuboid_vertices.push_back(topRightH);
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)), 1.0f));
//	}
//	//wall(3)
//	cuboid_vertices.push_back(topRight);
//	cuboid_vertices.push_back(topRightH);
//	cuboid_vertices.push_back(topLeft);
//	cuboid_vertices.push_back(topLeft);
//	cuboid_vertices.push_back(topRightH);
//	cuboid_vertices.push_back(topLeftH);
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)), 1.0f));
//	}
//	//wall(4)
//	cuboid_vertices.push_back(topLeft);
//	cuboid_vertices.push_back(topLeftH);
//	cuboid_vertices.push_back(lowerLeft);
//	cuboid_vertices.push_back(lowerLeft);
//	cuboid_vertices.push_back(topLeftH);
//	cuboid_vertices.push_back(lowerLeftH);
//	for (int i = 0;i < 6;i++) {
//		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f)), 1.0f));
//	}
//	for (int i = 0; i < cuboid_vertices.size(); i++) {
//		cuboid_colors.push_back(glm::vec4(0.8f, 0.6f, 1.0f, 1.0f));
//	}
//
//	glGenVertexArrays(1, &cuboid_VAO);
//	glBindVertexArray(cuboid_VAO);
//
//	glGenBuffers(3, &cuboid_VBO[0]);
//	//first buffer
//	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, cuboid_vertices.size() * sizeof(glm::vec4), &cuboid_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(0);
//	//Second array
//	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, cuboid_colors.size() * sizeof(glm::vec4), &cuboid_colors[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(1);
//	//thrid array
//	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[2]);
//	glBufferData(GL_ARRAY_BUFFER, cuboid_normal.size() * sizeof(glm::vec4), &cuboid_normal[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glBindVertexArray(0);
//
//}
//void CreateNormLines(void) {
//	for (int i = 0; i < cuboid_vertices.size(); i += 3) {
//		glm::vec4 a = cuboid_vertices[i];
//		glm::vec4 b = cuboid_vertices[i + 1];
//		glm::vec4 c = cuboid_vertices[i + 2];
//		glm::vec3 sum = glm::vec3(a) + glm::vec3(b) + glm::vec3(c);
//		glm::vec4 center(sum / 3.0f, 1.0f);
//		norm_vertices.push_back(center);
//
//		norm_vertices.push_back(center + cuboid_normal[i] * 0.1f);
//	}
//	
//	for (int i = 0; i < norm_vertices.size(); i++) {
//		norm_colors.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
//	}
//	glGenVertexArrays(1, &norm_VAO);
//	glBindVertexArray(norm_VAO); 
//
//	glGenBuffers(2, &norm_VBO[0]); 
//
//	// first buffer: vertex coordinates
//	glBindBuffer(GL_ARRAY_BUFFER, norm_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, norm_vertices.size() * sizeof(glm::vec4), &norm_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0); 
//	glEnableVertexAttribArray(0);
//
//	// second buffer: colors
//	glBindBuffer(GL_ARRAY_BUFFER, norm_VBO[1]); 
//	glBufferData(GL_ARRAY_BUFFER, norm_colors.size() * sizeof(glm::vec4), &norm_colors[0], GL_STATIC_DRAW); 
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0); 
//	glEnableVertexAttribArray(1);
//
//	glBindVertexArray(0);
//}
//
//void CreateFloor(void) {
//	glm::vec4 bottomLeft(-10.0f, 0.0f, -10.0f, 1.0f);
//	glm::vec4 bottomRight(10.0f, 0.0f, -10.0f, 1.0f);
//	glm::vec4 topLeft(-10.0f, 0.0f, 10.0f, 1.0f);
//	glm::vec4 topRight(10.0f, 0.0f, 10.0f, 1.0f);
//	
//	floor_vertices.push_back(bottomLeft);
//	floor_vertices.push_back(topLeft);
//	floor_vertices.push_back(bottomRight);
//	floor_vertices.push_back(bottomRight);
//	floor_vertices.push_back(topLeft);
//	floor_vertices.push_back(topRight);
//
//	
//	for (int i = 0; i < floor_vertices.size();i++) {
//		floor_colors.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
//		floor_normal.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
//	}
//	glGenVertexArrays(1, &floor_VAO);
//	glBindVertexArray(floor_VAO);
//
//	glGenBuffers(3, &floor_VBO[0]);
//	//first buffer
//	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, floor_vertices.size() * sizeof(glm::vec4), &floor_vertices[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(0);
//	//Second array
//	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, floor_colors.size() * sizeof(glm::vec4), &floor_colors[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(1);
//	//thrid array
//	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[2]);
//	glBufferData(GL_ARRAY_BUFFER, floor_normal.size() * sizeof(glm::vec4), &floor_normal[0], GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glBindVertexArray(0);
//
//}
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
	glBindVertexArray( 0 );

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

	//
	// Consider calling a function to create your object here
	//
	CreateSphere(0.0f,0.7f,0.0f,0.7f);
	CreateSphere(1.0f, 0.5f, 1.0f,0.5f); //(x,y,z,radius)
	CreateSphere(3.0f, 0.0f, 3.0f, 0.1f);
	CreateCylinder(-1.0f, 0.0f, 1.0f,0.5f,1.0f);// (x,y,z,radius, height)
	CreateCone(0.0f, 0.0f, -1.5f, 0.5f, 1.0f);// (x,y,z,radius, height)
	CreateCuboid(-1.5f, 0.0f, -1.5f, 1.0f, 1.5f, 1.25f);//(x,y,z,width,length,height)

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

	// Do program initialization
	init();

	// Enter the main loop
	glutMainLoop();

	return EXIT_SUCCESS;
}
