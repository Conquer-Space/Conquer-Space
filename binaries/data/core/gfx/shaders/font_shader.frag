#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    float tex = texture(text, TexCoords).r;
    if(tex == 0) {
        discard;
    }
    color = vec4(textColor, tex);
}
