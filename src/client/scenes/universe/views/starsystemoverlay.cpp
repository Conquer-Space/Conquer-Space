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

#include <glm/gtc/matrix_transform.hpp>

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
    varray.emplace_back(glm::vec4(-75.0060, 41.7128, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(-74.0060, 40.7128, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(-118.2426f, 34.0549, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(-119.2426f, 35.0549, 0.0f, 1.0f));
    line_mesh = std::make_shared<engine::Mesh>();
    glGenBuffers(1, &line_mesh->VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, line_mesh->VBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, varray.size() * sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &line_mesh->VAO);
    glBindVertexArray(line_mesh->VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, line_mesh->VBO);

    GLsizei N = (GLsizei)varray.size() - 2;
    line_mesh->indicies = 6 * (N - 1);
    line_mesh->buffer_type = engine::DrawType::ARRAYS;
}

void StarSystemOverlay::Ui() {}

void StarSystemOverlay::Update() {
    return;
    for (auto&& [body, texture] : universe.view<PlanetTexture>().each()) {
        if (texture.overlay == nullptr) {
            continue;
        }
        cqsp::engine::FramebufferTexture* overlay = texture.overlay;
        overlay->BeginDraw();
        glm::mat4 project;
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float aspect = (float)app.GetWindowWidth() / (float)app.GetWindowHeight();
        project = glm::ortho(-180.f, 180.f, 90.0f, -90.0f);

        line_shader->UseProgram();
        line_shader->setVec2("resolution", overlay->width, overlay->height);
        glm::mat4 modelview1(1.0f);

        glm::mat4 mvp1 = project * modelview1;
        line_shader->setMat4("mvp", mvp1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, line_mesh->VBO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        line_mesh->Draw();
        overlay->EndDraw();
        overlay->Resolve();
    }
    glViewport(0, 0, app.GetWindowWidth(), app.GetWindowHeight());
}
}  // namespace cqsp::client::systems
