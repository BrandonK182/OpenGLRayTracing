#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "Shape.h"
#include <cmath>

float pi = 3.14159;

float n = 20.0f;
void CreateSphere(float x, float y, float z, float r) {
	float thetaDif, phiDif;
	thetaDif = phiDif = 2 * pi / n;

	for (float phi = 0; phi < 2 * pi; phi += phiDif) {
		for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
			float Ax = r * cos(theta) * sin(phi);
			float Ay = r * sin(theta) * sin(phi);
			float Az = r * cos(phi);

			float A_x = r * cos(theta) * sin(phi + phiDif);
			float A_y = r * sin(theta) * sin(phi + phiDif);
			float A_z = r * cos(phi + phiDif);

			float Bx = r * cos(theta + thetaDif) * sin(phi);
			float By = r * sin(theta + thetaDif) * sin(phi);
			float Bz = r * cos(phi);

			float B_x = r * cos(theta + thetaDif) * sin(phi + phiDif);
			float B_y = r * sin(theta + thetaDif) * sin(phi + phiDif);
			float B_z = r * cos(phi + phiDif);

			glm::vec4 A(Ax + x, Ay + y, Az + z, 1.0f);
			glm::vec4 A_(A_x + x, A_y + y, A_z + z, 1.0f);
			glm::vec4 B(Bx + x, By + y, Bz + z, 1.0f);
			glm::vec4 B_(B_x + x, B_y + y, B_z + z, 1.0f);

			sphere_vertices.push_back(A);
			sphere_vertices.push_back(A_);
			sphere_vertices.push_back(B);
			sphere_vertices.push_back(B);
			sphere_vertices.push_back(A_);
			sphere_vertices.push_back(B_);

			//normal
			glm::vec3 directionA(Ax, Ay, Az);
			glm::vec3 directionA_(A_x, A_y, A_z);
			glm::vec3 directionB(Bx, By, Bz);
			glm::vec3 directionB_(B_x, B_y, B_z);

			glm::vec4 normalA(glm::normalize(directionA), 1.0f);
			glm::vec4 normalA_(glm::normalize(directionA_), 1.0f);
			glm::vec4 normalB(glm::normalize(directionB), 1.0f);
			glm::vec4 normalB_(glm::normalize(directionB_), 1.0f);


			sphere_normal.push_back(normalA);
			sphere_normal.push_back(normalA_);
			sphere_normal.push_back(normalB);
			sphere_normal.push_back(normalB);
			sphere_normal.push_back(normalA_);
			sphere_normal.push_back(normalB_);
		}
	}


	for (int i = 0; i < sphere_vertices.size(); i++) {
		sphere_colors.push_back(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
	}

	glGenVertexArrays(1, &sphere_VAO);
	glBindVertexArray(sphere_VAO);

	glGenBuffers(3, &sphere_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(glm::vec4), &sphere_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sphere_colors.size() * sizeof(glm::vec4), &sphere_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sphere_normal.size() * sizeof(glm::vec4), &sphere_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}


void CreateCylinder(float x, float y, float z, float r, float height) {
	float thetaDif = 2 * pi / n;
	//top and bottom
	for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
		float Ax = r * cos(theta);
		float Ay = 0.0f;
		float Az = r * sin(theta);
		float A_x = r * cos(theta + thetaDif);
		float A_y = 0.0f;
		float A_z = r * sin(theta + thetaDif);
		//bottom
		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + 0.01f + y, Az + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + 0.01f + y, A_z + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(0.0f + x, 0.01f + y, 0.0f + z, 1.0f));
		//top
		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y + height, A_z + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + y + height, Az + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(0.0f + x, height + y, 0.0f + z, 1.0f));
		//wall
		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y, A_z + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + y, Az + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + height + y, Az + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(Ax + x, Ay + height + y, Az + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y + height, A_z + z, 1.0f));
		cylinder_vertices.push_back(glm::vec4(A_x + x, A_y + y, A_z + z, 1.0f));

		//normal



		glm::vec4 normalA(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f);
		glm::vec4 normalB(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f);
		glm::vec4 normalC(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f);

		cylinder_normal.push_back(normalA);
		cylinder_normal.push_back(normalB);
		cylinder_normal.push_back(normalC);

		normalA = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
		normalB = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
		normalC = glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);

		cylinder_normal.push_back(normalA);
		cylinder_normal.push_back(normalB);
		cylinder_normal.push_back(normalC);



		normalA = glm::vec4(glm::normalize(glm::vec3(A_x, A_y, A_z)), 1.0f);
		normalB = glm::vec4(glm::normalize(glm::vec3(Ax, Ay, Az)), 1.0f);


		cylinder_normal.push_back(normalA);
		cylinder_normal.push_back(normalB);
		cylinder_normal.push_back(normalB);
		cylinder_normal.push_back(normalB);
		cylinder_normal.push_back(normalA);
		cylinder_normal.push_back(normalA);



	}
	for (int i = 0;i < cylinder_vertices.size();i++) {
		cylinder_colors.push_back(glm::vec4(0.6f, 1.0f, 0.6f, 1.0f));
	}
	glGenVertexArrays(1, &cylinder_VAO);
	glBindVertexArray(cylinder_VAO);

	glGenBuffers(3, &cylinder_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertices.size() * sizeof(glm::vec4), &cylinder_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, cylinder_colors.size() * sizeof(glm::vec4), &cylinder_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, cylinder_normal.size() * sizeof(glm::vec4), &cylinder_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void CreateCone(float x, float y, float z, float r, float height) {
	float thetaDif = 2 * pi / n;
	for (float theta = 0; theta < 2 * pi; theta += thetaDif) {
		float Ax = r * cos(theta);
		float Ay = 0.0f;
		float Az = r * sin(theta);
		float A_x = r * cos(theta + thetaDif);
		float A_y = 0.0f;
		float A_z = r * sin(theta + thetaDif);

		glm::vec4 A(Ax + x, Ay + 0.01f + y, Az + z, 1.0f);
		glm::vec4 A_(A_x + x, A_y + 0.01f + y, A_z + z, 1.0f);
		//bottom
		cone_vertices.push_back(A);
		cone_vertices.push_back(A_);
		cone_vertices.push_back(glm::vec4(0.0f + x, 0.01f + y, 0.0f + z, 1.0f));

		glm::vec4 topCenter(0.0f + x, height + y, 0.0f + z, 1.0f);
		//wall
		cone_vertices.push_back(A_);
		cone_vertices.push_back(A);
		cone_vertices.push_back(topCenter);

		//normal
		//bottom
		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
		cone_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));

		//wall
		glm::vec3 directionA(Ax, Ay, Az);
		glm::vec3 directionB(A_x, A_y, A_z);
		glm::vec3 directionC(0.0f, height, 0.0f);

		cone_normal.push_back(glm::vec4(glm::normalize(directionB), 1.0f));
		cone_normal.push_back(glm::vec4(glm::normalize(directionA), 1.0f));
		cone_normal.push_back(glm::vec4(glm::normalize(directionC), 1.0f));


	}

	for (int i = 0; i < cone_vertices.size();i++) {
		cone_colors.push_back(glm::vec4(1.0f, 0.6f, 0.2f, 1.0f));
	}
	glGenVertexArrays(1, &cone_VAO);
	glBindVertexArray(cone_VAO);

	glGenBuffers(3, &cone_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, cone_vertices.size() * sizeof(glm::vec4), &cone_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, cone_colors.size() * sizeof(glm::vec4), &cone_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, cone_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, cone_normal.size() * sizeof(glm::vec4), &cone_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}
void CreateCuboid(float x, float y, float z, float width, float length, float height) {
	glm::vec4 lowerLeft(-width / 2 + x, y + 0.0f, -length / 2 + z, 1.0f);
	glm::vec4 lowerRight(width / 2 + x, y + 0.0f, -length / 2 + z, 1.0f);
	glm::vec4 topLeft(-width / 2 + x, y + 0.0f, length / 2 + z, 1.0f);
	glm::vec4 topRight(width / 2 + x, y + 0.0f, length / 2 + z, 1.0f);
	glm::vec4 lowerLeftH(-width / 2 + x, y + height, -length / 2 + z, 1.0f);
	glm::vec4 lowerRightH(width / 2 + x, y + height, -length / 2 + z, 1.0f);
	glm::vec4 topLeftH(-width / 2 + x, y + height, length / 2 + z, 1.0f);
	glm::vec4 topRightH(width / 2 + x, y + height, length / 2 + z, 1.0f);

	//floor
	cuboid_vertices.push_back(lowerLeft);
	cuboid_vertices.push_back(lowerRight);
	cuboid_vertices.push_back(topLeft);
	cuboid_vertices.push_back(topLeft);
	cuboid_vertices.push_back(lowerRight);
	cuboid_vertices.push_back(topRight);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), 1.0f));
	}
	//ceiling
	cuboid_vertices.push_back(lowerLeftH);
	cuboid_vertices.push_back(topLeftH);
	cuboid_vertices.push_back(lowerRightH);
	cuboid_vertices.push_back(lowerRightH);
	cuboid_vertices.push_back(topLeftH);
	cuboid_vertices.push_back(topRightH);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f));
	}
	//wall(1)
	cuboid_vertices.push_back(lowerLeft);
	cuboid_vertices.push_back(lowerLeftH);
	cuboid_vertices.push_back(lowerRight);
	cuboid_vertices.push_back(lowerRight);
	cuboid_vertices.push_back(lowerLeftH);
	cuboid_vertices.push_back(lowerRightH);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)), 1.0f));
	}
	//wall(2)
	cuboid_vertices.push_back(lowerRight);
	cuboid_vertices.push_back(lowerRightH);
	cuboid_vertices.push_back(topRight);
	cuboid_vertices.push_back(topRight);
	cuboid_vertices.push_back(lowerRightH);
	cuboid_vertices.push_back(topRightH);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)), 1.0f));
	}
	//wall(3)
	cuboid_vertices.push_back(topRight);
	cuboid_vertices.push_back(topRightH);
	cuboid_vertices.push_back(topLeft);
	cuboid_vertices.push_back(topLeft);
	cuboid_vertices.push_back(topRightH);
	cuboid_vertices.push_back(topLeftH);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)), 1.0f));
	}
	//wall(4)
	cuboid_vertices.push_back(topLeft);
	cuboid_vertices.push_back(topLeftH);
	cuboid_vertices.push_back(lowerLeft);
	cuboid_vertices.push_back(lowerLeft);
	cuboid_vertices.push_back(topLeftH);
	cuboid_vertices.push_back(lowerLeftH);
	for (int i = 0;i < 6;i++) {
		cuboid_normal.push_back(glm::vec4(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f)), 1.0f));
	}
	for (int i = 0; i < cuboid_vertices.size(); i++) {
		cuboid_colors.push_back(glm::vec4(0.8f, 0.6f, 1.0f, 1.0f));
	}

	glGenVertexArrays(1, &cuboid_VAO);
	glBindVertexArray(cuboid_VAO);

	glGenBuffers(3, &cuboid_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, cuboid_vertices.size() * sizeof(glm::vec4), &cuboid_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, cuboid_colors.size() * sizeof(glm::vec4), &cuboid_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, cuboid_normal.size() * sizeof(glm::vec4), &cuboid_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

}
void CreateNormLines(void) {
	for (int i = 0; i < cuboid_vertices.size(); i += 3) {
		glm::vec4 a = cuboid_vertices[i];
		glm::vec4 b = cuboid_vertices[i + 1];
		glm::vec4 c = cuboid_vertices[i + 2];
		glm::vec3 sum = glm::vec3(a) + glm::vec3(b) + glm::vec3(c);
		glm::vec4 center(sum / 3.0f, 1.0f);
		norm_vertices.push_back(center);

		norm_vertices.push_back(center + cuboid_normal[i] * 0.1f);
	}

	for (int i = 0; i < norm_vertices.size(); i++) {
		norm_colors.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
	}
	glGenVertexArrays(1, &norm_VAO);
	glBindVertexArray(norm_VAO);

	glGenBuffers(2, &norm_VBO[0]);

	// first buffer: vertex coordinates
	glBindBuffer(GL_ARRAY_BUFFER, norm_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, norm_vertices.size() * sizeof(glm::vec4), &norm_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);

	// second buffer: colors
	glBindBuffer(GL_ARRAY_BUFFER, norm_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, norm_colors.size() * sizeof(glm::vec4), &norm_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void CreateFloor(void) {
	glm::vec4 bottomLeft(-10.0f, 0.0f, -10.0f, 1.0f);
	glm::vec4 bottomRight(10.0f, 0.0f, -10.0f, 1.0f);
	glm::vec4 topLeft(-10.0f, 0.0f, 10.0f, 1.0f);
	glm::vec4 topRight(10.0f, 0.0f, 10.0f, 1.0f);

	floor_vertices.push_back(bottomLeft);
	floor_vertices.push_back(topLeft);
	floor_vertices.push_back(bottomRight);
	floor_vertices.push_back(bottomRight);
	floor_vertices.push_back(topLeft);
	floor_vertices.push_back(topRight);


	for (int i = 0; i < floor_vertices.size();i++) {
		floor_colors.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		floor_normal.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	}
	glGenVertexArrays(1, &floor_VAO);
	glBindVertexArray(floor_VAO);

	glGenBuffers(3, &floor_VBO[0]);
	//first buffer
	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, floor_vertices.size() * sizeof(glm::vec4), &floor_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);
	//Second array
	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, floor_colors.size() * sizeof(glm::vec4), &floor_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	//thrid array
	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, floor_normal.size() * sizeof(glm::vec4), &floor_normal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

}