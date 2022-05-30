// Sort of a generic shader for all 3d objects.
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec4 aTangent;

out vec4 frag_pos;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec4 Tangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords = aTexCoords;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
    frag_pos = gl_Position;
}
