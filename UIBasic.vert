#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 offset;
layout (location = 1) in mat4 transform;
layout (location = 5) in float visibleOnScreen;

out float visible;
out vec3 color;
out float instance_ID;

void main()
{
	//vec4 pos = transforms[gl_InstanceID] * vec4(aPos, 1.0f);
	//vec4 pos = perspectiveProj * inverse(cameraTrans) * transform * vec4(aPos, 1.0f);
	//vec4 pos = cameraTrans * transform * vec4(aPos, 1.0f);
	vec4 pos = transform * vec4(aPos, 1.0f);
	//vec4 pos = vec4(aPos, 1.0f);

//	if(testPos.x != pos.x){
//		color = vec3(1.0f);
//	}
//	else{
//		color = vec3(0.8f, 0.8f, 0.0f);
//	}
	visible = visibleOnScreen;
	color = vec3(pos.x, pos.y, pos.z);
	gl_Position = pos;
	instance_ID = float(gl_InstanceID);
}