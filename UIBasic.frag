#version 330 core

in float visible;
in vec3 color;
in float instance_ID;

uniform float entitiesCount;
uniform int lastSelectedUI;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 ColourBuffer;
layout (location = 2) out int thisEntityID;

void main()
{

	if(visible != 1.0f)
		discard;

   //FragColor = vec4(abs(color), 1.0f);
   if(lastSelectedUI == int(instance_ID))
   {
	   FragColor = vec4(1.0f, 0.64f, 0.0f, 1.0f);
   }
   else{
		FragColor = vec4(color, 1.0f);
   }
   ColourBuffer = vec4(vec3(instance_ID / entitiesCount), 1.0f);
   thisEntityID = int(instance_ID);

}