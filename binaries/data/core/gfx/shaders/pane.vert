#version 330 core
layout (location = 0) in vec3 iPos;
layout (location = 1) in vec2 iTexCoord;

uniform mat4 transform;

out vec2 TexCoord;

void main()
{
    gl_Position = transform * vec4(iPos, 1.0);
    TexCoord = vec2(iTexCoord.x, iTexCoord.y);
}
