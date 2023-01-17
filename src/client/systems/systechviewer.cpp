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
#include "client/systems/systechviewer.h"

#include <vector>

#include "client/systems/gui/systooltips.h"
#include "common/components/player.h"
#include "common/components/science.h"
#include "common/util/nameutil.h"

namespace cqsp::client::systems {
void SysTechnologyViewer::Init() {}

void SysTechnologyViewer::DoUI(int delta_time) {
    using common::components::science::TechnologicalProgress;
    // Display UI
    auto view = GetUniverse().view<common::components::Player>();
    entt::entity player = view.front();
    ImGui::Begin("Technology Information");
    if (GetUniverse().any_of<TechnologicalProgress>(player)) {
        auto& progress = GetUniverse().get<TechnologicalProgress>(player);
        for (entt::entity researched : progress.researched_techs) {
            ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), researched));
        }
    } else {
        ImGui::Text("Nope, no technology");
    }
    ImGui::End();
}

void SysTechnologyViewer::DoUpdate(int delta_time) {}

void SysTechnologyProjectViewer::Init() {}

void SysTechnologyProjectViewer::DoUI(int delta_time) {
    using common::components::science::ScientificResearch;
    // Display UI
    auto view = GetUniverse().view<common::components::Player>();
    entt::entity player = view.front();
    ImGui::Begin("Technology Research");
    if (GetUniverse().any_of<ScientificResearch>(player)) {
        auto& progress = GetUniverse().get<ScientificResearch>(player);
        for (auto& researched : progress.current_research) {
            ImGui::TextFmt("{} {}", common::util::GetName(GetUniverse(), researched.first), researched.second);
        }

        ImGui::Separator();
        ImGui::Text("Potential Research");

        std::vector<entt::entity> potential_research;
        for (const entt::entity& researched : progress.potential_research) {
            ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), researched));
            ImGui::SameLine();
            if (ImGui::Button(fmt::format("Queue Research##{}", researched).c_str())) {
                // Add to tech queue
                potential_research.push_back(researched);
            }
        }
        for (entt::entity res : potential_research) {
            progress.potential_research.erase(res);
            progress.current_research[res] = 0;
        }
    } else {
        ImGui::Text("No Tech Research");
    }
    ImGui::End();
}

void SysTechnologyProjectViewer::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
