/*
 * Copyright 2021 Conquer Space
 */
#include "engine/renderer/primitives/pane.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cqspe = conquerspace::engine;
void conquerspace::primitive::MakeTexturedPaneMesh(cqspe::Renderable& renderable) {
    renderable.mesh = new cqspe::Mesh();
    MakeTexturedPaneMesh(*renderable.mesh);
}

void conquerspace::primitive::MakeTexturedPaneMesh(conquerspace::engine::Mesh& mesh) {
    float vertices[] = {
        // positions // texture coords
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top right
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
       -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
       -1.0f, -1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    mesh.VAO = VAO;
    mesh.VBO = VBO;
    mesh.EBO = EBO;
    mesh.indicies = 6;
    mesh.RenderType = GL_TRIANGLES;
}

void conquerspace::primitive::MakeTexturedPaneMesh(cqspe::Mesh& mesh, bool mirrored) {
        float vertices[] = {
        // positions // texture coords
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // top right
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // bottom right
       -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // bottom left
       -1.0f, -1.0f, 0.0f, 0.0f, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    mesh.VAO = VAO;
    mesh.VBO = VBO;
    mesh.EBO = EBO;
    mesh.indicies = 6;
    mesh.RenderType = GL_TRIANGLES;
}
