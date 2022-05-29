// Sort of a generic shader for all 3d objects.
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec4 aTangent;

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
    float C = 0.0001;
    float far = 9.461e12;
    gl_Position.z = 2.0*log(gl_Position.w*C + 1)/log(far*C + 1) - 1;

    gl_Position.z *= gl_Position.w;
}
