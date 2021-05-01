#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    float tex = texture(text, TexCoords).r;
    if(tex < 0.1) {
        discard;
    }
    vec4 sampled = vec4(1.0, 1.0, 1.0, tex);
    color = vec4(textColor, 1.0) * sampled;
}
