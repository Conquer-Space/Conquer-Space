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
#include "engine/renderer/renderer.h"

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

#include "engine/enginelogger.h"
#include "engine/glfwdebug.h"

namespace cqsp::engine {

using asset::ShaderProgram_t;
using asset::Texture;

void Draw(Renderable &renderable) {
    renderable.shaderProgram->UseProgram();
    int i = -1;
    for (std::vector<Texture *>::iterator it = renderable.textures.begin(); it != renderable.textures.end(); ++it) {
        i++;
        if ((*it)->texture_type == -1) {
            ENGINE_LOG_WARN("Texture {} is not initialized properly", (*it)->id);
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture((*it)->texture_type, (*it)->id);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            ENGINE_LOG_ERROR("Error when binding texture {}: {}", (*it)->id, engine::GetGlError(error));
        }
    }

    renderable.mesh->Draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void Draw(Renderable &renderable, ShaderProgram_t &shader) { Draw(renderable, shader.get()); }

void Draw(Renderable &renderable, asset::ShaderProgram *shader) {
    shader->UseProgram();
    int i = -1;
    for (std::vector<Texture *>::iterator it = renderable.textures.begin(); it != renderable.textures.end(); ++it) {
        i++;
        if ((*it)->texture_type == -1) {
            ENGINE_LOG_WARN("Texture {} is not initialized properly", (*it)->id);
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture((*it)->texture_type, (*it)->id);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            ENGINE_LOG_ERROR("Error when binding texture {}: {}", (*it)->id, engine::GetGlError(error));
        }
    }

    renderable.mesh->Draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

BasicRendererObject MakeRenderable() { return std::make_shared<Renderable>(); }

BasicRenderer::~BasicRenderer() = default;

void BasicRenderer::Draw() {
    // Then iterate through them and render
    for (const auto &renderable : renderables) {
        renderable->shaderProgram->UseProgram();
        renderable->shaderProgram->setMat4("model", renderable->model);
        renderable->shaderProgram->setMat4("view", view);
        renderable->shaderProgram->setMat4("projection", projection);
        int i = 0;
        for (std::vector<cqsp::asset::Texture *>::iterator it = renderable->textures.begin();
             it != renderable->textures.end(); ++it) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture((*it)->texture_type, (*it)->id);
            i++;
        }

        renderable->mesh->Draw();

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }
}
}  // namespace cqsp::engine