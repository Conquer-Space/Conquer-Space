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

#include <vector>

#include <glm/vec3.hpp>

#include "engine/graphics/mesh.h"

namespace cqsp::engine::primitive {
Mesh_t CreateFilledCircle(int segments = 64);
Mesh_t CreateFilledTriangle();
Mesh_t CreateFilledSquare();
Mesh_t CreateLineCircle(int segments = 64, float size = 1.0f);
Mesh_t CreateLineSequence(const std::vector<glm::vec3>& sequence);
}  // namespace cqsp::engine::primitive
