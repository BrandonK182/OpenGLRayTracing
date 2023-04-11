#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <vector>

//sphere
extern GLuint sphere_VAO;
extern GLuint sphere_VBO[3];
extern std::vector<glm::vec4> sphere_vertices;
extern std::vector<glm::vec4> sphere_colors;
extern std::vector<glm::vec4> sphere_normal;
extern GLuint floor_VAO;
extern GLuint floor_VBO[3];
extern std::vector<glm::vec4> floor_vertices;
extern std::vector<glm::vec4> floor_colors;
extern std::vector<glm::vec4> floor_normal;
extern GLuint cylinder_VAO;
extern GLuint cylinder_VBO[3];
extern std::vector<glm::vec4> cylinder_vertices;
extern std::vector<glm::vec4> cylinder_colors;
extern std::vector<glm::vec4> cylinder_normal;
extern GLuint cone_VAO;
extern GLuint cone_VBO[3];
extern std::vector<glm::vec4> cone_vertices;
extern std::vector<glm::vec4> cone_colors;
extern std::vector<glm::vec4> cone_normal;
extern GLuint cuboid_VAO;
extern GLuint cuboid_VBO[3];
extern std::vector<glm::vec4> cuboid_vertices;
extern std::vector<glm::vec4> cuboid_colors;
extern std::vector<glm::vec4> cuboid_normal;
extern GLuint norm_VAO;
extern GLuint norm_VBO[2];
extern std::vector<glm::vec4> norm_vertices;
extern std::vector<glm::vec4> norm_colors;

void CreateSphere(float x, float y, float z, float r);
void CreateCylinder(float x, float y, float z, float r, float height);
void CreateCone(float x, float y, float z, float r, float height);
void CreateCuboid(float x, float y, float z, float width, float length, float height);
void CreateNormLines(void);
void CreateFloor(void);


#endif 