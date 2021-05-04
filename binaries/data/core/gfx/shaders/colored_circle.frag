#version 330 core

in vec2 TexCoords;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}
