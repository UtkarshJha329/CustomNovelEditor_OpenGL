#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 perspectiveProj;
uniform mat4 cameraTrans;
uniform vec3 camPos;
uniform float lineWidth;

void build_line(vec3 posA, vec3 posB, vec3 cameraPos)
{    
    vec3 dir = posB - posA;
    vec3 center = (posB + posA) / 2;
    vec3 dirCamera = cameraPos - center;
    vec3 normal = cross(dirCamera, dir);
    normal = normalize(normal);
    normal *= lineWidth;

    dir = normalize(dir);
    dir *= 1.414;
    posA = posA + dir;
    posB = posB - dir;

    vec4 position = vec4(0.0f);

    position = perspectiveProj * inverse(cameraTrans) * vec4(posA + normal, 1.0f);
    gl_Position = position;    // 1:bottom-left
    EmitVertex();   

    position = perspectiveProj * inverse(cameraTrans) * vec4(posA - normal, 1.0f);
    gl_Position = position;    // 2:bottom-right
    EmitVertex();

    position = perspectiveProj * inverse(cameraTrans) * vec4(posB + normal, 1.0f);
    gl_Position = position;    // 3:top-left
    EmitVertex();

    position = perspectiveProj * inverse(cameraTrans) * vec4(posB - normal, 1.0f);
    gl_Position = position;    // 4:top-right
    EmitVertex();
    EndPrimitive();
}

void main() {    
    build_line(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), camPos);
}  