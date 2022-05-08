#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main() {
    FragColor = vec4(Normal, 1.0);
}
