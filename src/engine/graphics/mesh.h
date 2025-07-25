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

namespace cqsp::engine {
enum DrawType { ARRAYS = 0, ELEMENTS = 1 };

class Mesh {
 public:
    Mesh();
    ~Mesh();

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    unsigned int indicies;

    unsigned int mode;

    DrawType buffer_type = DrawType::ELEMENTS;

    void Draw();

    static void Destroy(Mesh& mesh);
};
typedef std::shared_ptr<Mesh> Mesh_t;
inline Mesh_t MakeMesh() { return std::make_shared<Mesh>(); }
}  // namespace cqsp::engine
