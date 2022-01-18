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
#include "engine/renderer/renderer.h"

#include <glad/glad.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

void cqsp::engine::Draw(Renderable &renderable) {
    renderable.shaderProgram->UseProgram();
    int i = -1;
    for (auto& texture : renderable.textures) {
        i++;
        if (texture->texture_type == -1) {
            SPDLOG_WARN("Texture {} is not initialized properly", texture->id);
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(texture->texture_type, texture->id);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            SPDLOG_ERROR("Error when binding texture {}: {}", texture->id, error);
        }
    }

    renderable.mesh->Draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

cqsp::engine::BasicRendererObject cqsp::engine::MakeRenderable() {
    return std::make_shared<cqsp::engine::Renderable>();
}

void cqsp::engine::BasicRenderer::Draw() {
    // Then iterate through them and render
    for (const auto& renderable : renderables) {
        renderable->shaderProgram->UseProgram();
        renderable->shaderProgram->setMat4("model", renderable->model);
        renderable->shaderProgram->setMat4("view", view);
        renderable->shaderProgram->setMat4("projection", projection);
        int i = 0;
        for (auto& texture : renderable->textures) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(texture->texture_type, texture->id);
            i++;
        }

        renderable->mesh->Draw();

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }
}
