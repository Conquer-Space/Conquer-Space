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
#include "sysstarsystemtree.h"

#include <string>

#include "client/scenes/universe/universescene.h"
#include "client/scenes/universe/views/starsystemrenderer.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/util/nameutil.h"
#include "engine/cqspgui.h"
#include "systooltips.h"

namespace cqsp::client::systems {

namespace components = core::components;
namespace bodies = components::bodies;
namespace types = components::types;
using bodies::OrbitalSystem;
using core::components::types::Orbit;
using core::util::GetName;
using ctx::VisibleOrbit;

void SysStarSystemTree::Init() {
    // Sort all the planets in order

    auto& orbital_system = GetUniverse().get<OrbitalSystem>(GetUniverse().sun);
    planets.emplace(GetUniverse().sun);
    planets.insert(orbital_system.bodies.begin(), orbital_system.bodies.end());
    planets.sort([&](const entt::entity lhs, const entt::entity rhs) {
        return (GetUniverse().get<types::Orbit>(lhs).semi_major_axis <
                GetUniverse().get<types::Orbit>(rhs).semi_major_axis);
    });
}

void SysStarSystemTree::DoUI(int delta_time) {
    // Get star system
    selected_planet = scene::GetCurrentViewingPlanet(GetUniverse());
    ImGui::SetNextWindowPos(ImVec2(30, ImGui::GetIO().DisplaySize.y - 30), ImGuiCond_Always, ImVec2(0.f, 1.f));
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_Always);
    ImGui::Begin("Star System", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | window_flags);
    if (ImGui::BeginTabBar("###sysstarsystemviewtabbar")) {
        if (ImGui::BeginTabItem("Star System Tree")) {
            MainTree();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Orbit Filter")) {
            OrbitFilter();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void SysStarSystemTree::DoUpdate(int delta_time) {}

void SysStarSystemTree::SeePlanetSelectable(entt::entity entity) {
    std::string planet_name = GetName(GetUniverse(), entity);
    bool is_selected = (entity == selected_planet);
    ImGui::Dummy(ImVec2(20, 16));
    ImGui::SameLine();
    if (CQSPGui::DefaultSelectable(planet_name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
        // Selected object
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            // Go to the planet
            scene::SeePlanet(GetUniverse(), entity);
        }
    }
    gui::EntityTooltip(GetUniverse(), entity);
}

void SysStarSystemTree::DoChildTree(entt::entity entity) {
    for (auto child : GetUniverse().get<OrbitalSystem>(entity).all()) {
        std::string child_name = GetName(GetUniverse(), child);
        bool is_selected = (child == selected_planet);
        if (!GetUniverse().any_of<OrbitalSystem>(child)) {
            // Then just selectable
            if (CQSPGui::DefaultSelectable(child_name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Go to the planet
                    scene::SeePlanet(GetUniverse(), child);
                }
            }
            gui::EntityTooltip(GetUniverse(), child);
        } else {
            if (ImGui::TreeNodeEx(child_name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
                // If it's double clicked
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Go to the planet
                    scene::SeePlanet(GetUniverse(), child);
                }
                // Get children
                gui::EntityTooltip(GetUniverse(), child);
                DoChildTree(child);
                ImGui::TreePop();
            } else {
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    scene::SeePlanet(GetUniverse(), child);
                }
                gui::EntityTooltip(GetUniverse(), child);
            }
        }
    }
}

void SysStarSystemTree::OrbitFilter() {
    // List out all the types of orbits and then determine if they are visible or not
    auto orbits = GetUniverse().view<Orbit>();
    if (ImGui::Checkbox("Hide all orbits", &hide_all_orbits)) {
        if (hide_all_orbits) {
            for (entt::entity orb : orbits) {
                GetUniverse().remove<VisibleOrbit>(orb);
            }
        } else {
            for (entt::entity orb : orbits) {
                GetUniverse().get_or_emplace<VisibleOrbit>(orb);
            }
        }
    }
    ImGui::BeginChild("orbitfiltercontainer", ImVec2(250, 400));
    if (ImGui::BeginTable("orbitfiltertable", 2)) {
        ImGui::TableSetupColumn("Orbit");
        ImGui::TableSetupColumn("Visible");
        ImGui::TableHeadersRow();
        int i = 0;
        for (entt::entity orb : orbits) {
            // Get the name
            if (GetUniverse().any_of<core::components::bodies::Planet>(orb)) {
                continue;
            }
            ImGui::TableNextRow();
            std::string name = core::util::GetName(GetUniverse(), orb);
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", name);
            ImGui::TableSetColumnIndex(1);
            bool check = GetUniverse().any_of<VisibleOrbit>(orb);
            ImGui::Checkbox(fmt::format("###Visible orbit{}", i).c_str(), &check);
            if (check) {
                GetUniverse().get_or_emplace<VisibleOrbit>(orb);
            } else {
                GetUniverse().remove<VisibleOrbit>(orb);
            }
            i++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void SysStarSystemTree::MainTree() {
    int index = 0;
    // Get selected planet
    // Sort by sma
    entt::entity current_planet = scene::GetCurrentViewingPlanet(GetUniverse());
    for (auto entity : planets) {
        if (!GetUniverse().any_of<OrbitalSystem>(entity) || entity == GetUniverse().sun) {
            SeePlanetSelectable(entity);
        } else {
            std::string planet_name = GetName(GetUniverse(), entity);
            if (ImGui::TreeNodeEx(planet_name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
                // If it's double clicked
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Go to the planet
                    scene::SeePlanet(GetUniverse(), entity);
                }
                // Get children
                gui::EntityTooltip(GetUniverse(), entity);
                DoChildTree(entity);
                ImGui::TreePop();
            } else {
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    scene::SeePlanet(GetUniverse(), entity);
                }
                gui::EntityTooltip(GetUniverse(), entity);
            }
        }
    }
}
}  // namespace cqsp::client::systems
