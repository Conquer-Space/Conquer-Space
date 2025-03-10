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
#include "engine/graphics/primitives/uvsphere.h"

#include <glad/glad.h>

#include <numbers>
#include <vector>

cqsp::engine::Mesh_t cqsp::engine::primitive::ConstructSphereMesh(int x_segments, int y_segments) {
    cqsp::engine::Mesh_t mesh = engine::MakeMesh();
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);

    unsigned int vbo;
    unsigned int ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;
    std::vector<unsigned int> indices;

    const float PI = std::numbers::pi;
    for (unsigned int y = 0; y <= y_segments; ++y) {
        for (unsigned int x = 0; x <= x_segments; ++x) {
            const float xSegment = static_cast<float>(x) / x_segments;
            const float ySegment = static_cast<float>(y) / y_segments;
            const float xTheta = xSegment * PI * 2;
            const float yTheta = ySegment * PI * 2;
            float xPos = std::cos(xTheta) * std::sin(yTheta / 2);
            float yPos = std::cos(yTheta / 2);
            float zPos = std::sin(xTheta) * std::sin(yTheta / 2);
            positions.emplace_back(xPos, yPos, zPos);
            // Invert x segments so that the texture shows up properly.
            uv.emplace_back(x_segments - xSegment, ySegment);
            normals.emplace_back(xPos, yPos, zPos);
            //tangents.push_back({std::cos(xTheta - PI / 2), 0.0f, std::sin(xTheta - PI / 2), 1.0f});
        }
    }

    for (unsigned int y = 0; y < y_segments; ++y) {
        if (y % 2 == 0) {
            for (unsigned int x = 0; x <= x_segments; ++x) {
                indices.push_back(y * (x_segments + 1) + x);
                indices.push_back((y + 1) * (x_segments + 1) + x);
            }
        } else {
            for (int x = x_segments; x >= 0; --x) {
                indices.push_back((y + 1) * (x_segments + 1) + x);
                indices.push_back(y * (x_segments + 1) + x);
            }
        }
    }

    std::vector<float> vertices;
    for (unsigned int i = 0; i < positions.size(); ++i) {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);
        if (!uv.empty()) {
            vertices.push_back(uv[i].x);
            vertices.push_back(uv[i].y);
        }
        if (!normals.empty()) {
            vertices.push_back(normals[i].x);
            vertices.push_back(normals[i].y);
            vertices.push_back(normals[i].z);
        }
        if (!tangents.empty()) {
            vertices.push_back(tangents[i].x);
            vertices.push_back(tangents[i].y);
            vertices.push_back(tangents[i].z);
            vertices.push_back(tangents[i].w);
        }
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    float stride = (3 + 2 + 3);
    if (!tangents.empty()) {
        stride += 4;
    }
    stride *= sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));  // NOLINT
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));  // NOLINT
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(5 * sizeof(float)));  // NOLINT
    if (!tangents.empty()) {
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(8 * sizeof(float)));  // NOLINT
    }

    mesh->VAO = vao;
    mesh->VBO = vbo;
    mesh->EBO = ebo;
    mesh->mode = GL_TRIANGLE_STRIP;
    mesh->indicies = indices.size();
    return mesh;
}
