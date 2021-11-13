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
#include "engine/graphics/primitives/polygon.h"

#include <glad/glad.h>

#include <vector>

#include <glm/glm.hpp>

#include "common/components/units.h"
#include "common/components/coordinates.h"

void cqsp::primitive::CreateFilledCircle(
    cqsp::engine::Mesh& mesh, int segments) {
    std::vector<float> positions;
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(0.5);
    positions.push_back(0.5);

    // Add texture coords
    for (int i = 0; i <= segments; i++) {
        double theta = i * cqsp::common::components::types::toRadian(360.f/segments);
        double y = std::sin(theta);
        double x = std::cos(theta);

        // Add positions
        positions.push_back(x);
        positions.push_back(y);
        positions.push_back(0);
        positions.push_back(0.5*x + 0.5);
        positions.push_back(0.5*-y + 0.5);
    }

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), &positions[0], GL_STATIC_DRAW);
    int stride = 5;
    glVertexAttribPointer(0, 3, GL_FLOAT,
                            GL_FALSE, stride * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float),
                            reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    mesh.VAO = vao;
    mesh.VBO = vbo;
    mesh.mode = GL_TRIANGLE_FAN;
    mesh.indicies = segments + 2;
    mesh.buffer_type = cqsp::engine::DrawType::ARRAYS;
}

void cqsp::primitive::CreateFilledTriangle(cqsp::engine::Mesh& renderable) {
    cqsp::primitive::CreateFilledCircle(renderable, 3);
}
void cqsp::primitive::CreateFilledSquare(cqsp::engine::Mesh& renderable) {
    cqsp::primitive::CreateFilledCircle(renderable, 4);
}
