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
#include "client/systems/sysstarsystemtree.h"

#include <string>

#include "client/systems/views/starsystemview.h"
#include "client/systems/gui/systooltips.h"
#include "client/scenes/universescene.h"
#include "common/components/bodies.h"
#include "common/components/name.h"

#include "engine/cqspgui.h"

void cqsp::client::systems::SysStarSystemTree::Init() {}

void cqsp::client::systems::SysStarSystemTree::DoUI(float delta_time) {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspcs = cqsp::client::systems;
    namespace cqspc = cqsp::common::components;
    // Get star system
    entt::entity ent = GetUniverse().view<cqspcs::RenderingStarSystem>().front();
    if (ent == entt::null) {
        return;
    }
    auto& star_system = GetUniverse().get<cqspb::StarSystem>(ent);
    ImGui::SetNextWindowPos(ImVec2(30, ImGui::GetIO().DisplaySize.y - 30),
                            ImGuiCond_Always, ImVec2(0.f, 1.f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Star System (Will add name soon)",
                nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | window_flags);
    int index = 0;
    // Get selected planet
    entt::entity current_planet = cqsp::scene::GetCurrentViewingPlanet(GetApp());
    for (auto entity : star_system.bodies) {
        bool is_selected = (entity == current_planet);
        std::string planet_name = fmt::format("{}", entity);
        if (GetUniverse().all_of<cqspc::Name>(entity)) {
            planet_name = fmt::format("{}", GetUniverse().get<cqspc::Name>(entity));
        }

        if (CQSPGui::DefaultSelectable(planet_name.c_str(), is_selected,
                                ImGuiSelectableFlags_AllowDoubleClick)) {
            // Selected object
            selected_index = index;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                // Go to the planet
                cqsp::scene::SeePlanet(GetApp(), entity);
            }
        }
        gui::EntityTooltip(GetUniverse(), entity);
        index++;
    }
    ImGui::End();
}

void cqsp::client::systems::SysStarSystemTree::DoUpdate(float delta_time) {}
