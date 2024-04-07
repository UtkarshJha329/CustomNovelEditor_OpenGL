#version 330 core

in vec3 color;
in float isVisible;
in float instance_ID;
in float down;
in vec3 posOut;

uniform float uiEntitiesCount;
uniform float entitiesCount;
uniform int lastSelectedEntity;
uniform int lastClickedEntity;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 ColourBuffer;
layout (location = 2) out int thisEntityID;

void main()
{
   	if(isVisible == 0.0f)
		discard;

   //FragColor = vec4(abs(color), 1.0f);

   vec4 finalColor;

   if(lastSelectedEntity == int(instance_ID) + int(uiEntitiesCount))
	{
		finalColor = vec4(1.0f, 0.8f, 0.86f, 1.0f);
	}
   else{
		//vec3 color = vec3(0.9f, 0.9f, 0.1f) * down;
		vec3 color = mix(vec3(1.0f), vec3(1.0f, 1.0f, 0.5f), down);

		if(lastClickedEntity == int(instance_ID) + int(uiEntitiesCount)){
			float borderStartAt = 0.97f;
			float border = (posOut.x < -borderStartAt || posOut.x > borderStartAt || posOut.y < -borderStartAt || posOut.y > borderStartAt) ? 0.0f : 1.0;
			color *= border;
		}

		finalColor = vec4(color, 1.0f);
   }



   FragColor = finalColor;
   ColourBuffer = vec4(vec3( (uiEntitiesCount + instance_ID) / entitiesCount), 1.0f);
   thisEntityID = int(uiEntitiesCount + instance_ID);

}