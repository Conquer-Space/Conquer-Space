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
#include "client/systems/syscommand.h"

#include <GLFW/glfw3.h>

#include <string>

#include "client/systems/sysstarsystemrenderer.h"
#include "client/systems/gui/systooltips.h"
#include "client/scenes/universescene.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/movement.h"
#include "common/components/ships.h"
#include "engine/cqspgui.h"

void conquerspace::client::systems::SysCommand::Init() {}

void conquerspace::client::systems::SysCommand::DoUI(int delta_time) {
    if (!to_see) {
        return;
    }

    namespace cqspb = conquerspace::common::components::bodies;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;
    namespace cqspcs = conquerspace::client::systems;
    // Get star system
    entt::entity ent = GetApp().GetUniverse().view<cqspcs::RenderingStarSystem>().front();
    if (ent == entt::null) {
        return;
    }
    auto& star_system = GetApp().GetUniverse().get<cqspb::StarSystem>(ent);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f,
                                   ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Order Target", &to_see,
                 ImGuiWindowFlags_NoResize | window_flags);
    int index = 0;
    // Get selected planet
    entt::entity current_planet = conquerspace::scene::GetCurrentViewingPlanet(GetApp());
    for (auto entity : star_system.bodies) {
        bool is_selected = (entity == current_planet);
        std::string planet_name = fmt::format("{}", entity);
        if (GetApp().GetUniverse().all_of<conquerspace::common::components::Name>(entity)) {
            planet_name = fmt::format(
                        "{}", GetApp().GetUniverse()
                        .get<conquerspace::common::components::Name>(entity)
                        .name);
        }

        if (CQSPGui::DefaultSelectable(GetApp(), planet_name.c_str(),
                        is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            // Selected object
            selected_index = index;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                // Go to the planet
                GetApp().GetUniverse().emplace_or_replace<cqspt::MoveTarget>(
                    current_planet, entity);
                SPDLOG_INFO("Move Ordered");
            }
        }
        gui::EntityTooltip(entity, GetApp().GetUniverse());
        index++;
    }
    ImGui::End();
}

void conquerspace::client::systems::SysCommand::DoUpdate(int delta_time) {
    namespace cqspb = conquerspace::common::components::bodies;
    namespace cqspt = conquerspace::common::components::types;
    selected_planet = conquerspace::scene::GetCurrentViewingPlanet(GetApp());
    entt::entity mouse_over =
        GetApp()
            .GetUniverse()
            .view<conquerspace::client::systems::MouseOverEntity,
                  cqspt::Kinematics>()
            .front();
    if (!ImGui::GetIO().WantCaptureMouse &&
        GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !conquerspace::scene::IsGameHalted() &&
        !GetApp().MouseDragged()) {
        to_see = true;
    }
    if (GetApp().GetUniverse().all_of<cqspt::Orbit>(selected_planet)) {
        to_see = false;
    }
}
