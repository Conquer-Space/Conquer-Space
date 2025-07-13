/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <map>
#include <string>

#include <glm/glm.hpp>

#include "engine/graphics/shader.h"

namespace cqsp::asset {
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;         // Size of glyph
    glm::ivec2 Bearing;      // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Horizontal offset to advance to next glyph
};

class Font : public Asset {
 public:
    std::map<unsigned char, Character> characters;
    unsigned int VAO, VBO;
    unsigned int texture;
    float initial_size;

    AssetType GetAssetType() override { return AssetType::FONT; }
};

void LoadFontData(Font& font, unsigned char* fontBuffer, uint64_t size);
void RenderText(cqsp::asset::ShaderProgram& shader, Font& font, std::string text, float x, float y, float scale,
                glm::vec3 color);
}  // namespace cqsp::asset
