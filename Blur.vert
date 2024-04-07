#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 transform;
layout (location = 5) in float visible;

out float isVisible;

uniform mat4 perspectiveProj;
uniform mat4 cameraTrans;


void main()
{
	vec4 pos = perspectiveProj * inverse(cameraTrans) * transform * vec4(aPos, 1.0f);

	isVisible = visible;

	gl_Position = pos;
}