/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <vector>

void conquerspace::engine::Draw(Renderable &renderable) {
    renderable.shaderProgram->UseProgram();
    int i = 0;
    for (std::vector<conquerspace::asset::Texture*>::iterator it = renderable.textures.begin();
                                                        it != renderable.textures.end(); ++it) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture((*it)->texture_type, (*it)->id);
        i++;
    }

    glBindVertexArray(renderable.mesh->VAO);
    if (renderable.mesh->buffer_type == 1) {
        glDrawElements(renderable.mesh->RenderType, renderable.mesh->indicies, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(renderable.mesh->RenderType, 0, renderable.mesh->indicies);
    }

    glBindVertexArray(0);

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

conquerspace::engine::BasicRendererObject conquerspace::engine::MakeRenderable() {
    return std::make_shared<conquerspace::engine::Renderable>();
}

void conquerspace::engine::BasicRenderer::Draw() {
    // Then iterate through them and render
    for (auto renderable : renderables) {
        renderable->shaderProgram->UseProgram();
        renderable->shaderProgram->setMat4("model", renderable->model);
        renderable->shaderProgram->setMat4("view", view);
        renderable->shaderProgram->setMat4("projection", projection);
        int i = 0;
        for (std::vector<conquerspace::asset::Texture*>::iterator it = renderable->textures.begin();
                                                    it != renderable->textures.end(); ++it) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture((*it)->texture_type, (*it)->id);
            i++;
        }

        glBindVertexArray(renderable->mesh->VAO);
        if (renderable->mesh->buffer_type == 1) {
            glDrawElements(renderable->mesh->RenderType, renderable->mesh->indicies,
                                                                        GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(renderable->mesh->RenderType, 0, renderable->mesh->indicies);
        }
        glBindVertexArray(0);

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }
}
