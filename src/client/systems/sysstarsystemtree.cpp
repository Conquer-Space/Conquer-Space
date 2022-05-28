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
#include "common/components/coordinates.h"
#include "common/components/name.h"

#include "engine/cqspgui.h"

namespace cqsp::client::systems {
void SysStarSystemTree::Init() {
    // Sort all the planets in order
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspt = cqsp::common::components::types;
    auto& orbital_system = GetUniverse().get<cqspb::OrbitalSystem>(GetUniverse().sun);
    planets.emplace(GetUniverse().sun);
    planets.insert(orbital_system.children.begin(),
                   orbital_system.children.end());
    planets.sort([&](const entt::entity lhs, const entt::entity rhs) {
        return (GetUniverse().get<cqspt::Orbit>(lhs).semi_major_axis <
                GetUniverse().get<cqspt::Orbit>(rhs).semi_major_axis);
    });
}

void SysStarSystemTree::DoUI(int delta_time) {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspcs = cqsp::client::systems;
    namespace cqspc = cqsp::common::components;
    // Get star system
    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    ImGui::SetNextWindowPos(ImVec2(30, ImGui::GetIO().DisplaySize.y - 30),
                            ImGuiCond_Always, ImVec2(0.f, 1.f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Star System", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | window_flags);
    int index = 0;
    // Get selected planet
    // Sort by sma
    entt::entity current_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    for (auto entity : planets) {
        if (!GetUniverse().any_of<cqspb::OrbitalSystem>(entity) || entity == GetUniverse().sun) {
            SeePlanetSelectable(entity); 
        } else {
            std::string planet_name = gui::GetName(GetUniverse(), entity);
            if (ImGui::TreeNodeEx(planet_name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
                // If it's double clicked
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Go to the planet
                    cqsp::scene::SeePlanet(GetApp(), entity);
                }
                // Get children
                gui::EntityTooltip(GetUniverse(), entity);
                DoChildTree(entity);
                ImGui::TreePop();
            } else {
                gui::EntityTooltip(GetUniverse(), entity);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Go to the planet
                    cqsp::scene::SeePlanet(GetApp(), entity);
                }
            }
        }
    }
    ImGui::End();
}

void SysStarSystemTree::DoUpdate(int delta_time) {}

void SysStarSystemTree::SeePlanetSelectable(entt::entity entity) {
    std::string planet_name = gui::GetName(GetUniverse(), entity);
    bool is_selected = (entity == selected_planet);
    ImGui::Dummy(ImVec2(20, 16));
    ImGui::SameLine();
    if (CQSPGui::DefaultSelectable(planet_name.c_str(), is_selected,
                                    ImGuiSelectableFlags_AllowDoubleClick)) {
        // Selected object
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            // Go to the planet
            cqsp::scene::SeePlanet(GetApp(), entity);
        }
    }
     gui::EntityTooltip(GetUniverse(), entity);
}

void SysStarSystemTree::DoChildTree(entt::entity entity) {
    namespace cqspb = cqsp::common::components::bodies;
    for (auto child : GetUniverse().get<cqspb::OrbitalSystem>(entity).children) {
        std::string child_name = gui::GetName(GetUniverse(), child);
        bool is_selected = (child == selected_planet);
        if (CQSPGui::DefaultSelectable(child_name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                // Go to the planet
                cqsp::scene::SeePlanet(GetApp(), child);
            }
        }
        gui::EntityTooltip(GetUniverse(), child);
    }}
}