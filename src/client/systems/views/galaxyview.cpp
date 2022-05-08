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
#include "client/systems/views/galaxyview.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/player.h"
#include "common/components/organizations.h"
#include "engine/graphics/primitives/polygon.h"
#include "engine/graphics/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"

using cqsp::client::systems::GalaxyRenderer;

GalaxyRenderer::GalaxyRenderer(cqsp::common::Universe &_u, cqsp::engine::Application &_a) :
                                                m_universe(_u), m_app(_a) {}

void GalaxyRenderer::Initialize() {
    circle_shader = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:2dcolorshader")->MakeShader();
    renderer = new cqsp::engine::Renderer2D(circle_shader, true);

    mesh = engine::primitive::CreateFilledCircle();
    star_system.mesh = engine::primitive::CreateFilledCircle();
    star_system.shaderProgram = circle_shader;
    previous_mouseX = m_app.GetMouseX();
    previous_mouseY = m_app.GetMouseY();
}

void GalaxyRenderer::OnTick() {}

void GalaxyRenderer::Render(float deltaTime) {
    namespace cqspc = cqsp::common::components;
    entt::entity player = m_universe.view<cqspc::Player>().front();
    entt::entity starting_planet = m_universe.get<cqspc::Civilization>(player).starting_planet;
    entt::entity home_system = m_universe.get<cqspc::bodies::Body>(starting_planet).star_system;
    // Render all star systems in galaxy
    // Just draw circles for now
    namespace cqspc = cqsp::common::components;
    auto view = m_universe.view<cqspc::bodies::StarSystem, cqspc::types::GalacticCoordinate>();
    for (entt::entity entity : view) {
        auto& coordinate = m_universe.get<cqspc::types::GalacticCoordinate>(entity);
        // Render on screen.
        // 2D coordinates for now.
        // Render based on coordinates
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(ConvertCoords(coordinate.x, coordinate.y), 1));

        const float size = 0.1;
        model = glm::scale(model, glm::vec3(size, size, 1));
        renderer->SetProjection(m_app.Get2DProj());
        glm::vec3 color = glm::vec3(1, 0, 0);
        if (entity == home_system) {
            color = glm::vec3(0, 1, 0);
        }

        renderer->DrawColoredSprite(mesh, color, ConvertCoords(coordinate.x, coordinate.y), glm::vec2(10, 10), 0);

        glm::vec2 v = ConvertCoords(coordinate.x, coordinate.y);
        m_app.DrawText(fmt::format("{}", entity), v.x, v.y);
    }
}

void GalaxyRenderer::Update(float deltaTime) {
    namespace cqspc = cqsp::common::components;
    // Check for zooming and clicking, and now we can view the star systems.
    if (!ImGui::GetIO().WantCaptureMouse) {
        double scrollBefore = scroll;
        double newScale = (exp(m_app.GetScrollAmount() * 0.1) * scroll);
        //Limit scale
        if (newScale > 1e-16 && newScale < 10) {
            scroll = newScale;
        }

        // Check for mouse drag
        if (m_app.MouseButtonIsHeld(engine::MouseInput::LEFT)) {
            view_x += (m_app.GetMouseX() - previous_mouseX) / scroll;
            view_y += (previous_mouseY - m_app.GetMouseY()) / scroll;
        }

        // Click on the star system
        if (m_app.MouseButtonDoubleClicked(engine::MouseInput::LEFT)) {
            mouse_over = entt::null;
            namespace cqspc = cqsp::common::components;
            float window_ratio = static_cast<float>(m_app.GetWindowWidth()) /
                                 static_cast<float>(m_app.GetWindowHeight());
            auto view = m_universe.view<cqspc::bodies::StarSystem, cqspc::types::GalacticCoordinate>();
            float x = m_app.GetMouseX();
            float y = m_app.GetWindowHeight() - m_app.GetMouseY();
            for (entt::entity entity : view) {
                auto& coordinate = m_universe.get<cqspc::types::GalacticCoordinate>(entity);
                glm::vec2 screen_pos = ConvertCoords(coordinate.x, coordinate.y);
                double posx = screen_pos.x;
                double posy = screen_pos.y;
                double t = glm::length(glm::vec2(x - posx, (y - posy)));
                if (t < 10) {
                    mouse_over = entity;
                    SPDLOG_INFO("Clicked on star system {}, switching to the star system", entity);
                    break;
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

glm::vec2 GalaxyRenderer::ConvertCoords(double x, double y) {
    return glm::vec2(((view_x + x) * scroll + m_app.GetWindowWidth()/2),
        ((view_y + y)) * scroll + m_app.GetWindowHeight()/2);
}
