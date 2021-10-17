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
#include "client/systems/galaxyrenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "engine/renderer/primitives/polygon.h"
#include "engine/renderer/renderer.h"

using cqsp::client::systems::GalaxyRenderer;

GalaxyRenderer::GalaxyRenderer(cqsp::common::Universe &_u, cqsp::engine::Application &_a) :
                                                m_universe(_u), m_app(_a) {}

void GalaxyRenderer::Initialize() {
    asset::ShaderProgram* circle_shader = m_app.GetAssetManager().
                CreateShaderProgram("core:shader.pane.vert", "core:coloredcirclefrag");

    star_system.mesh = new cqsp::engine::Mesh();
    primitive::CreateFilledCircle(*star_system.mesh);
    star_system.shaderProgram = circle_shader;
}

void GalaxyRenderer::OnTick() {}

void GalaxyRenderer::Render(float deltaTime) {
    // Render all star systems in galaxy
    // 
    float window_ratio = static_cast<float>(m_app.GetWindowWidth()) /
                         static_cast<float>(m_app.GetWindowHeight());
    // Just draw circles for now
    namespace cqspc = cqsp::common::components;
    auto view = m_universe.view<cqspc::bodies::StarSystem, cqspc::types::GalacticCoordinate>();
    for (entt::entity entity : view) {
        auto& coordinate = m_universe.get<cqspc::types::GalacticCoordinate>(entity);
        // Render on screen.
        // 2D coordinates for now.
        // Render based on coordinates
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(coordinate.x / 500.f - 1, coordinate.y / 500.f - 1, 0.f));
        model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
        model = glm::scale(model, glm::vec3(1, window_ratio, 1));

        glm::mat4 projection = glm::mat4(1.0f);
        star_system.shaderProgram->UseProgram();
        star_system.shaderProgram->setVec4("color", 1, 0, 0, 1);
        star_system.shaderProgram->setMat4("model", model);
        star_system.shaderProgram->setMat4("projection", projection);
        engine::Draw(star_system);
    }
}

void GalaxyRenderer::Update(float deltaTime) {
}

void GalaxyRenderer::DoUI(float deltaTime) {}
