// Basic shader without any lighting, and just displays the color input
// [uniform] color color output
#version 330 core

out vec4 FragColor;

uniform vec4 color;
uniform float C;
uniform float far;
uniform float offset;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in vec4 frag_pos;

void main()
{
    FragColor = color;
    gl_FragDepth = (log(C * frag_pos.z + offset) / log(C * far + offset));
}
