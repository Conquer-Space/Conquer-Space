/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/mesh.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

conquerspace::engine::Mesh::Mesh() {
    RenderType = GL_TRIANGLES;
}

conquerspace::engine::Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
