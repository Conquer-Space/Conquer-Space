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
#include "renderer2d.h"

#include <glad/glad.h>

#include <glm/gtx/transform.hpp>

cqsp::engine::Renderer2D::Renderer2D(cqsp::asset::ShaderProgram* tex, cqsp::asset::ShaderProgram* color) : texture_shader(tex), color_shader(color) {
}

void cqsp::engine::Renderer2D::DrawTexturedSprite(cqsp::engine::Mesh* mesh,
                                                  cqsp::asset::Texture& texture,
                                                 glm::vec2 position,
                                                 glm::vec2 size, float rotate) {
    texture_shader->UseProgram();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));

    texture_shader->Set("model", model);
    // Render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glBindVertexArray(mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void cqsp::engine::Renderer2D::DrawColoredSprite(cqsp::engine::Mesh* mesh,
                                                 glm::vec3 color,
                                                 glm::vec2 position,
                                                 glm::vec2 size, float rotate) {
    color_shader->UseProgram();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));

    texture_shader->Set("model", model);
    texture_shader->Set("color", color);


    glBindVertexArray(mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
