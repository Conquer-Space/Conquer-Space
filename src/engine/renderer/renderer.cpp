/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/renderer.h"

#include<glad/glad.h>
#include<glfw/glfw3.h>

void conquerspace::engine::Draw(Renderable &renderable) {
    renderable.shaderProgram->UseProgram();
    for (unsigned int i = 0; i < renderable.textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, renderable.textures.at(i)->id);
    }

    glBindVertexArray(renderable.mesh->VAO);
    glDrawElements(renderable.mesh->RenderType, renderable.mesh->indicies, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}
