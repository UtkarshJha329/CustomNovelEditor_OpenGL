#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 ColourBuffer;
layout (location = 2) out int thisEntityID;

in vec2 oTexCoords;
in vec3 color;
in float isVisible;

uniform sampler2D textureAtlas;

const float width = 0.5;
const float edge = 0.1;

void main()
{
	if(isVisible == 0.0f)
		discard;

	float distance = 1.0 - texture(textureAtlas, oTexCoords).a;
	float alpha = 1.0 - smoothstep(width, width + edge, distance);

	FragColor = vec4(vec3(0.0), alpha);
	//FragColor = vec4(color, 1.0f);
	//ColourBuffer = vec4(1.0f);
	//thisEntityID = int(-1.0f);
}