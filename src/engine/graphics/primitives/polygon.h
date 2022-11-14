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

#include <glm/vec3.hpp>

#include "engine/graphics/mesh.h"

namespace cqsp {
namespace engine::primitive {
cqsp::engine::Mesh* CreateFilledCircle(int segments = 64);
cqsp::engine::Mesh* CreateFilledTriangle();
cqsp::engine::Mesh* CreateFilledSquare();
cqsp::engine::Mesh* CreateLineCircle(int segments = 64, float size = 1.0f);
cqsp::engine::Mesh* CreateLineSequence(const std::vector<glm::vec3>& sequence);
}  // namespace engine::primitive
}  // namespace cqsp
