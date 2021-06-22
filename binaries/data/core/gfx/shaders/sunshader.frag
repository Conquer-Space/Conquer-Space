#version 330 core
out vec4 FragColor;

uniform vec4 color;
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{
    FragColor = color;
}
