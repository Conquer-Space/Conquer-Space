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
#include "orbitfilter.h"

#include <string>

#include "client/components/clientctx.h"
#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/util/nameutil.h"

void cqsp::client::systems::SysOrbitFilter::Init() {
    // Set all orbits to be visible
    auto orbits = GetUniverse().view<common::components::types::Orbit>();
    for (entt::entity orb : orbits) {
        GetUniverse().emplace_or_replace<ctx::VisibleOrbit>(orb);
    }
}

void cqsp::client::systems::SysOrbitFilter::DoUI(int delta_time) {
    if (!visible) {
        return;
    }
    ImGui::Begin("Orbit filter", &visible);
    // List out all the types of orbits and then determine if they are visible or not
    auto orbits = GetUniverse().view<common::components::types::Orbit>();
    if (ImGui::Checkbox("Hide all orbits", &hide_all_orbits)) {
        if (hide_all_orbits) {
            for (entt::entity orb : orbits) {
                GetUniverse().remove<ctx::VisibleOrbit>(orb);
            }
        } else {
            for (entt::entity orb : orbits) {
                GetUniverse().get_or_emplace<ctx::VisibleOrbit>(orb);
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
            if (GetUniverse().any_of<cqsp::common::components::bodies::Planet>(orb)) {
                continue;
            }
            ImGui::TableNextRow();
            std::string name = common::util::GetName(GetUniverse(), orb);
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", name);
            ImGui::TableSetColumnIndex(1);
            bool check = GetUniverse().any_of<ctx::VisibleOrbit>(orb);
            ImGui::Checkbox(fmt::format("###Visible orbit{}", i).c_str(), &check);
            if (check) {
                GetUniverse().get_or_emplace<ctx::VisibleOrbit>(orb);
            } else {
                GetUniverse().remove<ctx::VisibleOrbit>(orb);
            }
            i++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

void cqsp::client::systems::SysOrbitFilter::DoUpdate(int delta_time) {}
