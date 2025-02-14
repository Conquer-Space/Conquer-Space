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

#include "engine/graphics/mesh.h"
#include "engine/graphics/shader.h"
#include "engine/graphics/texture.h"

namespace cqsp {
namespace engine {
class Renderer2D {
 public:
    Renderer2D(cqsp::asset::ShaderProgram_t tex, cqsp::asset::ShaderProgram_t color);

    /// <summary>
    /// Only enable the texture shader
    /// </summary>
    /// <param name="tex"></param>
    explicit Renderer2D(cqsp::asset::ShaderProgram_t tex);

    /// <summary>
    /// Only enable the color shader
    /// </summary>
    /// <param name="color"></param>
    /// <param name=""></param>
    Renderer2D(cqsp::asset::ShaderProgram_t color, bool);

    void DrawTexturedSprite(cqsp::engine::Mesh* mesh, cqsp::asset::Texture& texture, glm::vec2 position, glm::vec2 size,
                            float rotate);
    void DrawColoredSprite(cqsp::engine::Mesh* mesh, glm::vec3 color, glm::vec2 position, glm::vec2 size, float rotate);

    void SetProjection(const glm::mat4& projection);

    bool TextureEnabled();
    bool ColorEnabled();

 private:
    cqsp::asset::ShaderProgram_t texture_shader;
    cqsp::asset::ShaderProgram_t color_shader;
};
}  // namespace engine
}  // namespace cqsp
