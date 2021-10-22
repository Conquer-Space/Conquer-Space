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
#include <glm/gtx/matrix_decompose.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "engine/renderer/primitives/polygon.h"
#include "engine/renderer/primitives/uvsphere.h"
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
    previous_mouseX = m_app.GetMouseX();
    previous_mouseY = m_app.GetMouseY();
}

void GalaxyRenderer::OnTick() {}

void GalaxyRenderer::Render(float deltaTime) {
    // Render all star systems in galaxy
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

        model = glm::translate(model, glm::vec3(((view_x + coordinate.x) / 500.f - 1) * scroll,
            ((view_y + coordinate.y) / 500.f - 1) * scroll, 1));
        model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::scale(projection, glm::vec3(1, window_ratio, 0));
        star_system.shaderProgram->UseProgram();
        star_system.shaderProgram->setVec4("color", 1, 0, 0, 1);
        star_system.shaderProgram->setMat4("model", model);
        star_system.shaderProgram->setMat4("projection", projection);
        engine::Draw(star_system);

        //glm::vec3((pos.x / m_app.GetWindowWidth() - 0.5) * 2,
            //py / m_app.GetWindowHeight() - 0.5) * 2 = ((view_x + coordinate.x) / 500.f - 1) * scroll
        // m_app.GetWindowHeight() * (scroll * (coordinate.x + view_x - 500) + 500)/1000

        // Draw text at location
        m_app.DrawTextNormalized(fmt::format("{}", entity), ((view_x + coordinate.x) / 500.f - 1) * scroll,
            ((view_y + coordinate.y) / 500.f - 1) * scroll * window_ratio);
    }
}

void GalaxyRenderer::Update(float deltaTime) {
    // Check for zooming and clicking, and now we can view the star systems.
    if (!ImGui::GetIO().WantCaptureMouse) {
        scroll -= m_app.GetScrollAmount() * 0.1;
        // Check for mouse drag

        if (m_app.MouseButtonIsHeld(GLFW_MOUSE_BUTTON_LEFT)) {
            view_x += m_app.GetMouseX() - previous_mouseX;
            view_y += previous_mouseY - m_app.GetMouseY();
            // Divide it by the window scale, somehow someway.
        }
        if (m_app.MouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT)) {
            // Click on the star system
            namespace cqspc = cqsp::common::components;
            float window_ratio = static_cast<float>(m_app.GetWindowWidth()) /
                         static_cast<float>(m_app.GetWindowHeight());
            auto view = m_universe.view<cqspc::bodies::StarSystem, cqspc::types::GalacticCoordinate>();
            for (entt::entity entity : view) {
                auto& coordinate = m_universe.get<cqspc::types::GalacticCoordinate>(entity);
                // Normalized device coordinates
                float x = (2.0f * m_app.GetMouseX()) / m_app.GetWindowWidth() - 1.0f;
                float y = 1.0f - (2.0f * m_app.GetMouseY()) / m_app.GetWindowHeight();
                // Now calculate the position of the thing
                double posx = view_x / m_app.GetWindowWidth() * scroll + (coordinate.x / 500.f - 1) * m_app.GetWindowWidth();
                double posy = view_y / m_app.GetWindowHeight() * scroll + (coordinate.y / 500.f - 1) * m_app.GetWindowHeight();
                double t = glm::length(glm::vec2(x - posx, y - posy));
                if (t < 0.01 * scroll) {
                    SPDLOG_INFO("{}", entity);
                }
            }
        }
        previous_mouseX = m_app.GetMouseX();
        previous_mouseY = m_app.GetMouseY();
    }
}

void GalaxyRenderer::DoUI(float deltaTime) {
    ImGui::Begin("Galaxy Renderer Debug window");
    ImGui::TextFmt("Offset: {} {}", view_x, view_y);
    ImGui::TextFmt("Scroll: {}", scroll);
    ImGui::End();
}
