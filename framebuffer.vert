#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 texCoords;

void main()
{
	gl_Position = vec4(inPos, 0.0f, 1.0f);
	texCoords = inTexCoords;

}