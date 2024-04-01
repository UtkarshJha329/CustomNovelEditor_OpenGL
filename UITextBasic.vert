#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 aTransform;
layout (location = 5) in mat4 aTexCoords;

out vec2 oTexCoords;
out vec3 color;

void main()
{
	gl_Position = aTransform * vec4(aPos, 1.0f);

	if(gl_VertexID == 3){										//TopRight
		oTexCoords = vec2(aTexCoords[3][0], aTexCoords[3][1]);
		color = vec3(1.0f, 0.0f, 0.0f);
	}
	else if(gl_VertexID == 2){									//Top Left
		oTexCoords = vec2(aTexCoords[2][0], aTexCoords[2][1]);
		color = vec3(0.0f, 1.0f, 0.0f);
	}
	else if(gl_VertexID == 1){									
		oTexCoords = vec2(aTexCoords[1][0], aTexCoords[1][1]);
		color = vec3(0.0f, 0.0f, 1.0f);
	}
	else{
		oTexCoords = vec2(aTexCoords[0][0], aTexCoords[0][1]);
		color = vec3(1.0f, 1.0f, 0.0f);
	}

	//oTexCoords = aTexCoordsB;
}