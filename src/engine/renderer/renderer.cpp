/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/renderer.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <algorithm>

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

conquerspace::engine::BasicRendererObject conquerspace::engine::MakeRenderable() {
    return std::make_shared<conquerspace::engine::Renderable>();
}

void conquerspace::engine::BasicRenderer::Draw() {
    //std::sort(renderables.begin(), renderables.end(), [](Renderable* i, Renderable* j) { return (i->z < j->z); });
    //Then iterate through them and render
    for (auto renderable : renderables) {
        renderable->shaderProgram->UseProgram();
        renderable->shaderProgram->setMat4("model", renderable->model);
        renderable->shaderProgram->setMat4("view", view);
        renderable->shaderProgram->setMat4("projection", projection);
        for (unsigned int i = 0; i < renderable->textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, renderable->textures.at(i)->id);
        }

        glBindVertexArray(renderable->mesh->VAO);
        glDrawElements(renderable->mesh->RenderType, renderable->mesh->indicies, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }
}
