#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;  

void main()
{
    vec4 col = texture(screenTexture, TexCoords);
    if (col.a <= 0) {
        discard;
    }
    FragColor = col;
}
