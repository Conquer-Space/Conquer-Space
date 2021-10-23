/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include "engine/renderer/shader.h"
#include "engine/renderer/texture.h"
#include "engine/renderer/mesh.h"

namespace cqsp {
namespace engine {
class Renderer2D {
   public:
    Renderer2D(cqsp::asset::ShaderProgram* tex, cqsp::asset::ShaderProgram* color);

    void DrawTexturedSprite(cqsp::engine::Mesh* mesh, cqsp::asset::Texture &texture, glm::vec2 position, glm::vec2 size, float rotate);
    void DrawColoredSprite(cqsp::engine::Mesh* mesh, glm::vec3 color, glm::vec2 position, glm::vec2 size, float rotate);

    cqsp::asset::ShaderProgram* texture_shader;
    cqsp::asset::ShaderProgram* color_shader;
};
}  // namespace engine
}  // namespace cqsp
