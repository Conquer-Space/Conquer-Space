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
#include "engine/renderer/renderer2d.h"

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

cqsp::engine::Renderer2D::Renderer2D(cqsp::asset::ShaderProgram_t tex,
                                     cqsp::asset::ShaderProgram_t color) :
                                     texture_shader(std::move(tex)), color_shader(std::move(color)) { }

cqsp::engine::Renderer2D::Renderer2D(cqsp::asset::ShaderProgram_t tex) :
                                        texture_shader(std::move(tex)), color_shader(nullptr) { }

cqsp::engine::Renderer2D::Renderer2D(cqsp::asset::ShaderProgram_t color, bool /*unused*/) :
                                        texture_shader(nullptr), color_shader(std::move(color)) { }


void cqsp::engine::Renderer2D::DrawTexturedSprite(cqsp::engine::Mesh* mesh,
                                                  cqsp::asset::Texture& texture,
                                                 glm::vec2 position,
                                                 glm::vec2 size, float rotate) {
    if (!TextureEnabled()) {
        return;
    }
    texture_shader->UseProgram();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    // Divide by two because for some reason, this thing renders things too big if we don't
    size /= 2;

    model = glm::scale(model, glm::vec3(size, 1.0f));

    texture_shader->Set("model", model);

    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    mesh->Draw();
}

void cqsp::engine::Renderer2D::SetProjection(const glm::mat4 & projection) {
    if (TextureEnabled()) {
        texture_shader->UseProgram();
        texture_shader->Set("projection", projection);
    }
    if (ColorEnabled()) {
        color_shader->UseProgram();
        color_shader->Set("projection", projection);
    }
}

void cqsp::engine::Renderer2D::DrawColoredSprite(cqsp::engine::Mesh* mesh,
                                                 glm::vec3 color,
                                                 glm::vec2 position,
                                                 glm::vec2 size, float rotate) {
    if (!ColorEnabled()) {
        return;
    }
    color_shader->UseProgram();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    size /= 2;

    model = glm::scale(model, glm::vec3(size, 1.0f));

    color_shader->Set("model", model);
    color_shader->Set("color", glm::vec4(color, 1));

    mesh->Draw();
}

bool cqsp::engine::Renderer2D::ColorEnabled() { return color_shader != nullptr; }
bool cqsp::engine::Renderer2D::TextureEnabled() { return texture_shader != nullptr; }
