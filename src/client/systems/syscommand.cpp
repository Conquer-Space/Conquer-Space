/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <sstream>
#include <string>
#include <vector>

#include "client/scenes/universe/interface/systooltips.h"
#include "client/scenes/universe/universescene.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/player.h"
#include "common/components/ships.h"
#include "engine/cqspgui.h"

namespace components = cqsp::common::components;
namespace bodies = components::bodies;
namespace ships = components::ships;
namespace types = components::types;
namespace systems = cqsp::client::systems;

using components::Name;

namespace cqsp::client::systems {
void SysCommand::Init() {}

void SysCommand::DoUI(int delta_time) {
    ShipList();
    if (!to_see) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f, ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Order Target", &to_see, ImGuiWindowFlags_NoResize | window_flags);
    int index = 0;
    // Get selected planet
    entt::entity current_planet = scene::GetCurrentViewingPlanet(GetUniverse());
    for (auto entity : GetUniverse().view<common::components::types::Orbit>()) {
        bool is_selected = (entity == current_planet);
        std::string planet_name = fmt::format("{}", entity);
        if (GetUniverse().all_of<Name>(entity)) {
            planet_name = fmt::format("{}", GetUniverse().get<Name>(entity).name);
        }

        if (CQSPGui::DefaultSelectable(planet_name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            // Selected object
            selected_index = index;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && selected_ship != entt::null) {
                // Go to the planet
                GetUniverse().emplace_or_replace<types::MoveTarget>(selected_ship, entity);
                SPDLOG_INFO("Move Ordered");
            }
        }
        gui::EntityTooltip(GetUniverse(), entity);
        index++;
    }
    ImGui::End();
}

void SysCommand::DoUpdate(int delta_time) {
    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    /*entt::entity mouse_over = GetUniverse()
            .view<cqsp::client::systems::MouseOverEntity, cqspt::Kinematics>().front();
    if (!ImGui::GetIO().WantCaptureMouse &&
        GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !GetApp().MouseDragged()) {
        to_see = true;
    }*/
}

void SysCommand::ShipList() {
    /*
    static entt::entity selectedFleetEnt = GetUniverse()
                           .get<cqspc::Civilization>(GetUniverse()
                           .view<cqspc::Player>()
                           .front()).top_level_fleet;

    auto& selectedFleet = GetUniverse().get<cqsps::Fleet>(selectedFleetEnt);
    auto& selectedFleetName = GetUniverse().get<cqspc::Name>(selectedFleetEnt);

    std::stringstream finalSelectedFleetName;
    for (size_t i = 0; i < selectedFleet.echelon; i++) {
        finalSelectedFleetName << GetUniverse()
                                      .get<cqsp::common::components::Name>(
                                          selectedFleet.parent_fleet)
                                      .name
                               << "/";
    }
    finalSelectedFleetName << selectedFleetName.name.c_str();

    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin(finalSelectedFleetName.str().c_str());

    // Show ship list
    int index = 0;
    static int selected = 0;

    for (entt::entity enti : selectedFleet.ships) {
        index++;
        const bool is_selected = (selected == index);
        std::string entity_name = cqsp::client::systems::gui::GetName(GetUniverse(), enti);
        if (CQSPGui::DefaultSelectable(entity_name.c_str(), is_selected)) {
            selected = index;
            to_see = true;
            selected_ship = enti;
        }
    }

    std::vector<entt::entity> subfleetsAndLast = selectedFleet.subfleets;
    bool has_parent = false;
    if (selectedFleet.parent_fleet != entt::null) {
        subfleetsAndLast.push_back(selectedFleet.parent_fleet);
        has_parent = true;
    }

    for (size_t i = 0; i < subfleetsAndLast.size(); i++) {
        index++;

        const bool is_selected = (selected == index);
        std::stringstream entity_name;
        entity_name << (i == (subfleetsAndLast.size() - 1) && has_parent
            ? "<-"
            : "->")
                    << cqsp::client::systems::gui::GetName(GetUniverse(),
                                                           subfleetsAndLast[i]);
        if (CQSPGui::DefaultSelectable(entity_name.str().c_str(),
                                       is_selected)) {
            selected = index;
            selectedFleetEnt = subfleetsAndLast[i];
        }
    }

    ImGui::End();*/
}
} // namespace cqsp::client::systems

