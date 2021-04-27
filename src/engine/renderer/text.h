/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <map>
#include <string>

#include <glm/glm.hpp>

#include "engine/renderer/shader.h"

namespace conquerspace {
namespace asset {
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Horizontal offset to advance to next glyph
};

class Font : public Asset {
 public:
    std::map<unsigned char, Character> characters;
    unsigned int VAO, VBO;
    unsigned int texture;
    float initial_size;
};

void LoadFont(Font& font, unsigned char *fontBuffer, uint64_t size);
void RenderText(conquerspace::asset::ShaderProgram &shader, Font& font,
                std::string text, float x, float y, float scale, glm::vec3 color);
}  // namespace asset
}  // namespace conquerspace
