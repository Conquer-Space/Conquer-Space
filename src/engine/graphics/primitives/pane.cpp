/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "engine/graphics/primitives/pane.h"

#include <glad/glad.h>

namespace cqspe = cqsp::engine;

cqsp::engine::Mesh* cqsp::engine::primitive::MakeTexturedPaneMesh() {
    cqsp::engine::Mesh* mesh = new cqsp::engine::Mesh();
    float vertices[] = {
        // positions // texture coords
        1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  // top right
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, 1.0f,  0.0f, 0.0f, 0.0f,  // bottom left
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(static_cast<uintptr_t>(0)));  // NOLINT
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(static_cast<uintptr_t>(3 * sizeof(float))));  // NOLINT
    glEnableVertexAttribArray(1);

    mesh->VAO = VAO;
    mesh->VBO = VBO;
    mesh->EBO = EBO;
    mesh->indicies = 6;
    mesh->mode = GL_TRIANGLES;
    return mesh;
}

cqsp::engine::Mesh* cqsp::engine::primitive::MakeTexturedPaneMesh(bool mirrored) {
    cqsp::engine::Mesh* mesh = new cqsp::engine::Mesh();

    float vertices[] = {
        // positions // texture coords
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // top right
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // bottom right
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // bottom left
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(0));  // NOLINT
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));  // NOLINT
    glEnableVertexAttribArray(1);

    mesh->VAO = VAO;
    mesh->VBO = VBO;
    mesh->EBO = EBO;
    mesh->indicies = 6;
    mesh->mode = GL_TRIANGLES;
    return mesh;
}
