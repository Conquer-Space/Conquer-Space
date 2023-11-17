#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;
uniform vec3 diffuse;

void main()
{    
    FragColor = vec4(diffuse, 1.0);//texture(diffuse, TexCoords);
}
