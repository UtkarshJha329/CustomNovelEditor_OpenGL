#pragma once

#include <glad/glad.h>

class VBO
{
public:
	VBO() {}
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(GLfloat* vertices, GLsizeiptr size, GLenum staticOrDynamic);

	void Init(GLfloat* vertices, GLsizeiptr size, GLenum staticOrDynamic) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, staticOrDynamic);
	}

	// Binds the VBO
	void Bind();
	// Unbinds the VBO
	void Unbind();
	// Deletes the VBO
	void Delete();
};