#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;
uniform sampler2D blurTexture;

vec4 CurrentColor(vec4 color1, vec4 color2){

	if(color1 == vec4(1.0f)){
		return color2;
	}else {
		return color1;
	}
}

void main()
{
	vec4 color = vec4(0.0);
    float blurSize = 2.0; // Adjust blur size as needed

    // Sample surrounding pixels and accumulate color
    for (float x = -blurSize; x <= blurSize; ++x) {
        for (float y = -blurSize; y <= blurSize; ++y) {
            vec2 offset = vec2(x, y) / vec2(800, 600);
            color += texture(blurTexture, texCoords + offset);
        }
    }

    // Average the accumulated color
    color = color * 1/25.0f;



   FragColor = CurrentColor(texture(screenTexture, texCoords), color);
   //FragColor = blurColor;
}