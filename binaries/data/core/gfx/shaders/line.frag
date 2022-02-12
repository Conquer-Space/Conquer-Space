// Basic shader without any lighting, and just displays the color input
// [uniform] color color output
#version 330 core

out vec4 FragColor;

uniform vec4 color;
in vec3 FragPos;

void main()
{
    FragColor = color;
}
