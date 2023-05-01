#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "shader.h"
#include "shaderprogram.h"

#include <vector>
#include <limits>
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

float maxDistance = 100;
int maxDepth = 3;

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
glm::vec4 lightSource;




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

std::vector<glm::vec4> vertices_all;

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

//scalar multiply vec3
glm::vec3 operator*(float scalar, const glm::vec3& v) {
	return glm::vec3(scalar * v.x, scalar * v.y, scalar * v.z);
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
bool rayTriangleIntersect(glm::vec3 &orig, glm::vec3 &dir, glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2, float &t,float &u, float &v) {
	//compute the plane that the triangle lies on's normal
	glm::vec3 A(v1 - v0);
	glm::vec3 B(v2 - v0);
	glm::vec3 pvec(glm::cross(dir, B));
	float det = glm::dot(A,pvec);


	//if the determinant is negative, triangle is back facing
	//if determinant is clsoe to 0, the ray misses the triangle
	if (det < DBL_MIN) { return false; };

	//ray and triangle are parallel if det is close to 0
	if (abs(det) < DBL_MIN) { return false; };

	float invDet = 1 / det;
	glm::vec3 tvec = orig - v0;
	u = glm::dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1) { return false; };

	glm::vec3 qvec(glm::cross(tvec, A));
	v = glm::dot(dir, qvec) * invDet;
	if (v < 0 || u + v > 1) { return false; };

	t = glm::dot(B, qvec) * invDet;

	return true;
}

bool intersect(glm::vec3 &orig,glm::vec3 &dir, float& tNear, int  &triIndex, glm::vec2& uv) {
	int j = 0;
	bool intersection = false;
	for (int i = 0; i < sphere_vertices.size() /3; ++i) {
		glm::vec3 v0 = sphere_vertices[j];
		glm::vec3 v1 = sphere_vertices[j + 1];
		glm::vec3 v2 = sphere_vertices[j + 2];
		float t = MAXINT, u, v;
		if (rayTriangleIntersect(orig, dir, v0, v1, v2, t, u, v) && t<tNear) {
			tNear = t;
			uv.x = u;
			uv.y = v;
			triIndex = i;
			intersection = true;
		}
		j += 3;
	}
	return intersection;
}
bool checkLight(glm::vec3 point, glm::vec3 light, glm::vec3 normal)
{
	glm::vec3 vector = light - point;

	//check if the light is the opposite direction to the triangle face
	//if (glm::dot(vector, normal) < 0)
	//{
		//return false;
	//}

	vector = glm::normalize(vector);
	float lowestT = maxDistance;

	int j = 0;
	for (int i = 0; i < sphere_normal.size(); i++)
	{
		//std::cout << "creating the vectors for each of the object" << std::endl;
		glm::vec3 v1 = sphere_vertices[j];
		glm::vec3 v2 = sphere_vertices[j+1];
		glm::vec3 v3 = sphere_vertices[j+2];

		//std::cout << "comparing to polygon " << i << std::endl;

		glm::vec3 A(v2 - v1);
		glm::vec3 B(v3 - v1);

		glm::vec3 C = cross(A, B);
		float D = -glm::dot(C, v1);

		//find the distance between camera and intersection point
		float dist = -(glm::dot(C, point) + D) / glm::dot(C, vector);

		//float t = (glm::dot(midpoints[i], normals[i]) - glm::dot(point, normals[i]) / (glm::dot(vector, normals[i])));
		glm::vec3 ray = point + (dist * vector);



		//std::cout << "checking if the point is in the triangle" << std::endl;
		if (PointInTriangle(ray, v1, v2, v3))
		{
			//std::cout << " CONNECTION!!!!!!!       point connects to triangle" << std::endl;
			//marks the ray as touching the triangle at some point
			//checks if the contact is the closest contact point
			if (dist < lowestT && dist > 0.01f)
			{
				//std::cout << "lower than max" << std::endl;
				return false;
			}
		}
		j += 3;
	}
	//does not intersect with any other triangles
	return true;
}
float max(float a, float b)
{
	return a > b ? a : b;
}
glm::vec4 shade(int triIndex) {
	glm::vec4 Ia(0.5, 0.5, 0.5, 1.0f);
	glm::vec4 Id(0.7, 0.7, 0.7, 1.0f);
	glm::vec4 Is(1.0, 1.0, 1.0, 1.0f);

	//ka & kd are vert color
	glm::vec4 ka, kd;
	ka = kd = sphere_colors[triIndex];
	glm::vec4 ks(1.0f, 1.0f, 1.0f, 1.0f);

	float shininess = 32.0f;

	glm::mat4 transf = PerspViewMatrix * PerspModelMatrix;

	glm::vec3 FragPos(transf * sphere_vertices[triIndex]);
	glm::vec3 FragNorm = glm::mat3(transpose(inverse(transf))) * sphere_normal[triIndex];
	glm::vec3 LightPos(transf * lightSource);

	glm::vec3 N = glm::normalize(FragNorm); // vertex normal
	glm::vec3 L = glm::normalize(LightPos - FragPos); // light direction
	glm::vec3 R = glm::normalize(reflect(-L, N)); // reflected ray
	glm::vec3 V = glm::normalize(glm::vec3(0.0, 0.0, 1.0)); // view direction

	float dotLN = dot(L, N);
	glm::vec4 amb = ka * Ia;
	glm::vec4 dif = kd * Id * dotLN;
	glm::vec4 spe = ks * Is * pow(max(dot(V, R), 0.0), shininess) * dotLN;

	return amb + dif + spe;
}
glm::vec3 RayTrace(glm::vec3 s, glm::vec3 dir, int depth) {

	bool intersection = false;
	glm::vec3 background_color(0.0f,0.0f,0.0f);

	float p_rg = 1.0f;
	float p_tg = 1.0f;

	float tNear;
	int triIndex;
	glm::vec2 uv;
	for (float t = 0.0f; t <= 10.0f; t+= 0.1f) {
		tNear = MAXINT;

		intersection = intersect(s, dir, tNear, triIndex, uv);
	}
	//if not intersected
	if (intersection == false) {
		return background_color;
	}
	glm::vec3 ray = s + (tNear * dir);
	glm::vec3 pixColor(sphere_colors[triIndex]);
	if (depth == maxDepth)
		return pixColor;
	
	if (checkLight(ray, lightSource, sphere_normal[triIndex]))
	{
		glm::vec3 I(shade(triIndex)); 

		float reflectivity = 0.33f;

		glm::vec3 u = dir;
		glm::vec3 n(sphere_vertices[triIndex]);
		glm::vec3 r = dir - 2 * glm::dot(u,n) * n;
		I = I + reflectivity * RayTrace(ray, r, depth + 1);
		return I;
	}
	return	pixColor * 0.5f;

}

void RayTraceMain() {
	glm::vec3 x(eyePos.x, eyePos.y, eyePos.z);  // let x be the postion of the viewer
	int maxDepth = 0;							// let maxDepth be a positive integer

	float length = 2.0f / 800.0f; //length of the pixel
	//For each pixel p in the viewport
	for (int i = 0; i < plane_vertices.size();i += 6) {
		//calculate the middle of the pixel p 
		glm::vec4 v0(plane_vertices[i]);
		//find the middle of the width and height of the pixel by dividing two
		float midpoint = length / 2.0f;
		glm::vec3 p(v0.x + midpoint , v0.y +midpoint, v0.z - midpoint); // found p

		//set u = unit vector in the direction from x to p (camera to pixel);
		glm::vec3 u(glm::normalize(p - x));
		//call RayTrace
		glm::vec3 color(RayTrace(x, u, maxDepth));
		//assign pixel p the color return by Ray Trace
		for (int j = 0; j < 6;j++) {
			plane_colors.push_back(glm::vec4(color,0.1f));
		}
	}

	//
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
	glm::vec3 cameraPos(eyePos.x,eyePos.y,eyePos.z);
	//vector of the plane
	glm::vec3 randomPoint(1.0f, 0.0f, 0.0f);
	glm::vec3 normalv1(glm::normalize(glm::cross(cameraPos, randomPoint)); 
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
	//RayTraceMain();

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

		/*eyepos.x = perspRotationX;
		eyepos.y = perspRotationY;
		CreatePlane();*/
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
	CreateSphere1(spherex, spherey, spherez, r);
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

	lightSource = glm::vec4(3.0f, 0.0f, 3.0f,1.0f);
	//
	// Consider calling a function to create your object here
	//
	//CreateSphere(0.0f,0.7f,0.0f,0.7f);
	//CreateSphere(1.0f, 0.5f, 1.0f,0.5f); //(x,y,z,radius)
	//CreateSphere(3.0f, 0.0f, 3.0f, 0.1f);
	//CreateCylinder(-1.0f, 0.0f, 1.0f,0.5f,1.0f);// (x,y,z,radius, height)
	//CreateCone(0.0f, 0.0f, -1.5f, 0.5f, 1.0f);// (x,y,z,radius, height)
	//CreateCuboid(-1.5f, 0.0f, -1.5f, 1.0f, 1.5f, 1.25f);//(x,y,z,width,length,height)
	//CreateCylinder(2.0f, 0.0f, 0.0f, 0.2f, 2.0f);

	CreateSphere1(0.0f, 0.5f, 0.0f, 0.5f);
	//CreateTriangle();


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
