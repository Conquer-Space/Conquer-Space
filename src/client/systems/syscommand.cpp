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

#include "client/systems/views/starsystemview.h"
#include "client/systems/gui/systooltips.h"
#include "client/scenes/universescene.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/coordinates.h"
#include "common/components/ships.h"
#include "engine/cqspgui.h"

void cqsp::client::systems::SysCommand::Init() {}

void cqsp::client::systems::SysCommand::DoUI(int delta_time) {
    ShipList();
    if (!to_see) {
        return;
    }

    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    namespace cqspcs = cqsp::client::systems;

    using cqsp::common::components::Name;
    // Get star system
    entt::entity ent = GetUniverse().view<cqspcs::RenderingStarSystem>().front();
    if (ent == entt::null) {
        return;
    }
    auto& star_system = GetUniverse().get<cqspb::StarSystem>(ent);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f,
                                   ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Order Target", &to_see,
                 ImGuiWindowFlags_NoResize | window_flags);
    int index = 0;
    // Get selected planet
    entt::entity current_planet = cqsp::scene::GetCurrentViewingPlanet(GetApp());
    for (auto entity : star_system.bodies) {
        bool is_selected = (entity == current_planet);
        std::string planet_name = fmt::format("{}", entity);
        if (GetUniverse().all_of<Name>(entity)) {
            planet_name = fmt::format("{}", GetUniverse().get<Name>(entity));
        }

        if (CQSPGui::DefaultSelectable(planet_name.c_str(),
                        is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            // Selected object
            selected_index = index;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && selected_ship!= entt::null) {
                // Go to the planet
                GetUniverse().emplace_or_replace<cqspt::MoveTarget>(selected_ship, entity);
                SPDLOG_INFO("Move Ordered");
            }
        }
        gui::EntityTooltip(GetUniverse(), entity);
        index++;
    }
    ImGui::End();
}

void cqsp::client::systems::SysCommand::DoUpdate(int delta_time) {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspt = cqsp::common::components::types;
    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetApp());
    /*entt::entity mouse_over = GetUniverse()
            .view<cqsp::client::systems::MouseOverEntity, cqspt::Kinematics>().front();
    if (!ImGui::GetIO().WantCaptureMouse &&
        GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !GetApp().MouseDragged()) {
        to_see = true;
    }*/
}

void cqsp::client::systems::SysCommand::ShipList() {
    namespace cqspcs = cqsp::client::systems;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    entt::entity ent = GetUniverse().view<cqspcs::RenderingStarSystem>().front();
    if (ent == entt::null) {
        return;
    }
    ImGui::Begin("Ships");
    auto& star_system = GetUniverse().get<cqspb::StarSystem>(ent);
    // Show ship list
    //ImGui::Begin
    int index = 0;
    static int selected = 0;

    for (entt::entity enti : star_system.bodies) {
        if (GetUniverse().all_of<cqsps::Ship>(enti)) {
            // Then do the things
            index++;
        } else {
            continue;
        }
        const bool is_selected = (selected == index);
        if (CQSPGui::DefaultSelectable(fmt::format("{}", enti).c_str(), is_selected)) {
            selected = index;
            to_see = true;
            selected_ship = enti;
        }
    }
    ImGui::End();
}
