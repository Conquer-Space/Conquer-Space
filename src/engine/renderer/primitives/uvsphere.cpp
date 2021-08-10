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

#include "engine/renderer/primitives/uvsphere.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

void cqsp::primitive::ConstructSphereMesh(
    int x_segments, int y_segments, cqsp::engine::Mesh& mesh) {
    GLuint VAO = 0;
    glGenVertexArrays(1, &VAO);

    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359;
    for (unsigned int y = 0; y <= y_segments; ++y) {
        for (unsigned int x = 0; x <= y_segments; ++x) {
            float xSegment = static_cast<float>(x) / x_segments;
            float ySegment = static_cast<float>(y) / y_segments;
            float xPos = std::cos(xSegment * PI * 2) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * PI * 2) * std::sin(ySegment * PI);

            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
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

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i) {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (uv.size() > 0) {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0) {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    float stride = (3 + 2 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT,
                            GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT,
                            GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT,
                            GL_FALSE, stride, reinterpret_cast<void*>(5 * sizeof(float)));

    mesh.VAO = VAO;
    mesh.VBO = vbo;
    mesh.EBO = ebo;
    mesh.RenderType = GL_TRIANGLE_STRIP;
    mesh.indicies = indices.size();
}
