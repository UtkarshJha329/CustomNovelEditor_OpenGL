#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>


std::string get_file_contents(const char* filename);

class Shader
{
public:
	unsigned int ID;
	Shader() { ID = 0; }
	Shader(const char* vertexFile, const char* fragmentFile);
	void Init(const char* vertexFile, const char* fragmentFile);
	void compileErrors(unsigned int shader, const char* type);
	void Activate();
	void Delete();
};