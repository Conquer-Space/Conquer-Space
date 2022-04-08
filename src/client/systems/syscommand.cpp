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

#include <sstream>
#include <vector>
#include <string>




#include "client/systems/views/starsystemview.h"
#include "client/systems/gui/systooltips.h"
#include "client/scenes/universescene.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/coordinates.h"
#include "common/components/ships.h"
#include "engine/cqspgui.h"
#include "common/components/player.h"

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
    namespace cqspc = cqsp::common::components;


    static entt::entity selectedFleetEnt = GetUniverse()
                           .get<cqspc::Civilization>(GetUniverse()
                           .view<cqspc::Player>()
                           .front()).topLevelFleet;

    auto& selectedFleet = GetUniverse().get<cqsps::Fleet>(selectedFleetEnt);
    auto& selectedFleetName = GetUniverse().get<cqspc::Name>(selectedFleetEnt);



    std::stringstream finalSelectedFleetName;
    for (size_t i = 0; i < selectedFleet.echelon; i++) {
        finalSelectedFleetName << GetUniverse()
                                      .get<cqsp::common::components::Name>(
                                          selectedFleet.parentFleet)
                                      .name
                               << "/";
    }
    finalSelectedFleetName << selectedFleetName.name.c_str();

    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin(finalSelectedFleetName.str().c_str());


    //show fleet list
    //we get the player



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
    std::vector<entt::entity> subfleetsAndLast = selectedFleet.subFleets;
    bool has_parent = false;
    if (selectedFleet.parentFleet != entt::null) {
        subfleetsAndLast.push_back(selectedFleet.parentFleet);
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

    ImGui::End();
}
