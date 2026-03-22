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
#include "client/scenes/universe/views/starsystemoverlay.h"

#include "client/components/planetrendering.h"
#include "core/components/bodies.h"

namespace cqsp::client::systems {
namespace bodies = core::components::bodies;
using asset::ShaderDefinition;
using client::components::PlanetTexture;

StarSystemOverlay::StarSystemOverlay(core::Universe& universe, engine::Application& app)
    : universe(universe), app(app) {}

void StarSystemOverlay::Initialize() {
    for (auto&& [body, body_comp] : universe.view<bodies::Body>().each()) {
        auto& data = universe.get_or_emplace<PlanetTexture>(body);
        data.overlay = new cqsp::engine::FramebufferTexture();
        cqsp::engine::FramebufferTexture* overlay = data.overlay;
        overlay->InitTexture(4096, 2048);
    }
    // Now also initialize shaders
    line_shader = app.GetAssetManager().GetAsset<ShaderDefinition>("core:shader.line")->MakeShader();
    // Init line
    std::vector<glm::vec4> varray;
    varray.emplace_back(glm::vec4(0.0f, -1.0f, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    for (int u = 0; u <= 90; u += 10) {
        double a = u * 3.1415926535 / 180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u = 90; u >= 0; u -= 10) {
        double a = u * 3.1415926535 / 180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c - 1.0f, (float)s - 1.0f, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    line_mesh = std::make_shared<engine::Mesh>();
    glGenBuffers(1, &line_mesh->VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, line_mesh->VBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, varray.size() * sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &line_mesh->VAO);
    glBindVertexArray(line_mesh->VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, line_mesh->VBO);

    GLsizei N = (GLsizei)varray.size() - 2;
    line_mesh->indicies = 6 * (N - 1);
}

void StarSystemOverlay::Update() {
    // Then we should draw it?
}
}  // namespace cqsp::client::systems
