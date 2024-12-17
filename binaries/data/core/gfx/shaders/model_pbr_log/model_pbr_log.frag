// Basic shader without any lighting, and just displays the color input
// [uniform] FragColor color output
#version 330 core

out vec4 FragColor;

uniform float C;
uniform float far;
uniform float offset;

uniform sampler2D diffuse;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in vec4 frag_pos;

void main()
{
    FragColor = vec4(texture(diffuse, TexCoords).rgb, 1);
    gl_FragDepth = (log(C * frag_pos.z + offset) / log(C * far + offset));
}
