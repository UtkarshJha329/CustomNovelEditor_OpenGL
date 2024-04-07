#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{
   //FragColor = texture(screenTexture, texCoords);

   
   vec2 texelSize = 1.0 / vec2(800, 600);
   vec4 blurColor = vec4(0.0f);
   for (int x = -5; x <= 5; x++) {
        for (int y = -5; y <= 5; y++) {
            vec2 offset = vec2(x, y) * texelSize;
                blurColor += texture(screenTexture, texCoords + offset);
        }
    }
        
   vec4 color = blurColor / 121.0; // Divide by the number of samples
   //color = vec4(color * 0.1, color.w);
   FragColor = color;
}