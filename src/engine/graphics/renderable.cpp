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
#include "engine/graphics/renderable.h"

void cqsp::engine::Renderable::SetTexture(const std::string& name, int id, asset::Texture* texture) {
    shaderProgram->UseProgram();
    shaderProgram->setInt(name, id);

    textures.push_back(texture);
}

void cqsp::engine::Renderable::SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    shaderProgram->UseProgram();
    shaderProgram->setMat4("model", model);
    shaderProgram->setMat4("view", view);
    shaderProgram->setMat4("projection", projection);
}
