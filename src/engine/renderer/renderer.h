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

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "engine/graphics/renderable.h"

namespace cqsp {
namespace engine {
void Draw(Renderable &);
void Draw(Renderable &, asset::ShaderProgram_t &shader);
void Draw(Renderable &, asset::ShaderProgram *shader);

typedef std::shared_ptr<Renderable> BasicRendererObject;

BasicRendererObject MakeRenderable();

class BasicRenderer {
 public:
    ~BasicRenderer();
    glm::mat4 projection = glm::mat4(1.0);
    glm::mat4 view = glm::mat4(1.0);
    std::vector<BasicRendererObject> renderables;
    void Draw();
};
}  // namespace engine
}  // namespace cqsp
