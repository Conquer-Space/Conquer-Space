/*
 * Copyright 2021 Conquer Space
*/
#include "engine/renderer/primitives/polygon.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include <glm/glm.hpp>

#include "common/components/orbit.h"


void conquerspace::primitive::CreateFilledCircle(
    conquerspace::engine::Mesh& mesh, int segments) {
    std::vector<float> positions;
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(0.5);
    positions.push_back(0.5);

    // Add texture coords
    for (int i = 0; i <= segments; i++) {
        double theta = i * conquerspace::common::components::types::toRadian(360.f/segments);
        double y = std::sin(theta);
        double x = std::cos(theta);

        // Add positions
        positions.push_back(x);
        positions.push_back(y);
        positions.push_back(0);
        positions.push_back(0.5*x + 0.5);
        positions.push_back(0.5*-y + 0.5);
    }

    GLuint VAO = 0;
    glGenVertexArrays(1, &VAO);

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), &positions[0], GL_STATIC_DRAW);
    int stride = 5;
    glVertexAttribPointer(0, 3, GL_FLOAT,
                            GL_FALSE, stride * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT,
                            GL_FALSE, stride * sizeof(float),
                            reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    mesh.VAO = VAO;
    mesh.VBO = vbo;
    mesh.RenderType = GL_TRIANGLE_FAN;
    mesh.indicies = segments+2;
    mesh.buffer_type = 0;
}

void conquerspace::primitive::CreateFilledTriangle(conquerspace::engine::Mesh& renderable) {
    conquerspace::primitive::CreateFilledCircle(renderable, 3);
}
void conquerspace::primitive::CreateFilledSquare(conquerspace::engine::Mesh& renderable) {
    conquerspace::primitive::CreateFilledCircle(renderable, 4);
}
