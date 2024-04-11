#pragma once
#include<glad/glad.h>

class EBO
{
public:
	EBO() {}
	// ID reference of Elements Buffer Object
	GLuint ID;
	// Constructor that generates a Elements Buffer Object and links it to indicies
	EBO(GLuint* indicies, GLsizeiptr size);

	void Init(GLuint* indicies, GLsizeiptr size) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indicies, GL_STATIC_DRAW);
	}

	// Binds the EBO
	void Bind();
	// Unbinds the EBO
	void Unbind();
	// Deletes the EBO
	void Delete();
};