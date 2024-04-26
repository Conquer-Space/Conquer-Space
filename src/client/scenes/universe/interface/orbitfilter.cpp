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
#include "orbitfilter.h"

#include <string>

#include "client/components/clientctx.h"
#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/util/nameutil.h"

using cqsp::client::systems::SysOrbitFilter;
using cqsp::common::components::types::Orbit;
using entt::entity;
using cqsp::client::ctx::VisibleOrbit;

void SysOrbitFilter::Init() {
    // Set all orbits to be visible
    for (entity orb : GetUniverse().view<Orbit>()) {
        GetUniverse().emplace_or_replace<VisibleOrbit>(orb);
    }
}

void SysOrbitFilter::DoUI(int delta_time) {
    if (!visible) {
        return;
    }
    ImGui::Begin("Orbit filter", &visible);
    // List out all the types of orbits and then determine if they are visible or not
    if (ImGui::Checkbox("Hide all orbits", &hide_all_orbits)) {
        auto orbits = GetUniverse().view<Orbit>();
        if (hide_all_orbits) {
            for (entity orb : orbits) {
                GetUniverse().remove<VisibleOrbit>(orb);
            }
        } else {
            for (entity orb : orbits) {
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
        for (entity orb : GetUniverse().view<Orbit>()) {
            // Get the name
            if (GetUniverse().any_of<common::components::bodies::Planet>(orb)) {
                continue;
            }
            ImGui::TableNextRow();
            std::string name = common::util::GetName(GetUniverse(), orb);
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
    ImGui::End();
}

void SysOrbitFilter::DoUpdate(int delta_time) {}
