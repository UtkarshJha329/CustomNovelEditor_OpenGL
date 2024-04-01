#include"VAO.h"

// Constructor that generates a VAO ID
VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}

// Links a VBO to the VAO using a certain layout
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

// Links a VBO to the VAO using a certain layout
void VAO::LinkTransformAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, size_t offset)
{
	VBO.Bind();
	int pos = layout;
	int pos1 = pos + 0;
	int pos2 = pos + 1;
	int pos3 = pos + 2;
	int pos4 = pos + 3;
	glEnableVertexAttribArray(pos1);
	glEnableVertexAttribArray(pos2);
	glEnableVertexAttribArray(pos3);
	glEnableVertexAttribArray(pos4);
	glVertexAttribPointer(pos1, numComponents, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*) (offset * 0));
	glVertexAttribPointer(pos2, numComponents, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*) (offset * 4));
	glVertexAttribPointer(pos3, numComponents, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*) (offset * 8));
	glVertexAttribPointer(pos4, numComponents, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*) (offset * 12));
	glVertexAttribDivisor(pos1, 1);
	glVertexAttribDivisor(pos2, 1);
	glVertexAttribDivisor(pos3, 1);
	glVertexAttribDivisor(pos4, 1);
	VBO.Unbind();
}


// Binds the VAO
void VAO::Bind()
{
	glBindVertexArray(ID);
}

// Unbinds the VAO
void VAO::Unbind()
{
	glBindVertexArray(0);
}

// Deletes the VAO
void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}