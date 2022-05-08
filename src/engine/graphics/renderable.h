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

#include <vector>
#include <string>

#include "engine/graphics/shader.h"
#include "engine/graphics/texture.h"
#include "engine/graphics/mesh.h"

namespace cqsp {
namespace engine {
class Renderable {
 public:
    Mesh* mesh;
    glm::mat4 model;

    cqsp::asset::ShaderProgram_t shaderProgram;
    std::vector<asset::Texture*> textures;

    void SetTexture(const std::string& name, int id, asset::Texture* texture);
    void SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

    int z = 0;
};
}  // namespace engine
}  // namespace cqsp
