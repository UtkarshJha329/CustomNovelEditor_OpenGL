#version 330 core

in vec3 color;
in float instance_ID;

uniform float uiEntitiesCount;
uniform float entitiesCount;
uniform int lastSelectedEntity;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 ColourBuffer;
layout (location = 2) out int thisEntityID;

void main()
{
   
   //FragColor = vec4(abs(color), 1.0f);

   if(lastSelectedEntity == int(instance_ID) + int(uiEntitiesCount))
   {
		FragColor = vec4(0.9f, 0.2f, 0.7f, 1.0f);
	}
   else{
		FragColor = vec4(0.8f, 0.0f, 0.8f, 1.0f);
   }

   ColourBuffer = vec4(vec3( (uiEntitiesCount + instance_ID) / entitiesCount), 1.0f);
   thisEntityID = int(uiEntitiesCount + instance_ID);

}