#pragma once
#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	// ID reference for the Vertex Array Object
	GLuint ID;
	// Constructor that generates a VAO ID
	VAO();

	// Links a VBO to the VAO using a certain layout
	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void LinkTransformAttrib(VBO& VBO, GLuint layout, GLuint numComponents = 4, GLenum type = GL_FLOAT, GLsizeiptr stride = sizeof(float) * 4 * 4, size_t offset = sizeof(float));
	// Binds the VAO
	void Bind();
	// Unbinds the VAO
	void Unbind();
	// Deletes the VAO
	void Delete();
};