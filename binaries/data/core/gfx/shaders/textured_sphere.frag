#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture0;

void main() {
    FragColor = vec4(texture(texture0, TexCoords).rgb, 1);
}
