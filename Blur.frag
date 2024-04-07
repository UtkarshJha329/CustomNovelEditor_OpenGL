#version 330 core

in float isVisible;

layout (location = 0) out vec4 FragColor;

void main()
{
   if(isVisible == 0.0f)
		discard;

	FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}