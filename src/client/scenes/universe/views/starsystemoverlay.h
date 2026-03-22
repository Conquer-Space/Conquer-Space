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

#include "core/universe.h"
#include "engine/application.h"
#include "engine/graphics/mesh.h"
#include "engine/graphics/shader.h"

namespace cqsp::client::systems {
class StarSystemOverlay {
 public:
    StarSystemOverlay(core::Universe &, engine::Application &);

    void Initialize();
    void Ui();
    void Update();

    std::vector<glm::vec4> GeneratePoints(const std::vector<glm::vec2> &points);

 private:
    core::Universe &universe;
    engine::Application &app;
    engine::Mesh_t line_mesh;
    asset::ShaderProgram_t line_shader;
    float value1 = 2.0;
    float value2 = 2.0;
};
}  // namespace cqsp::client::systems
