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

#include "common/components/player.h"
#include "common/components/science.h"

#include "client/systems/gui/systooltips.h"

namespace cqsp::client::systems {
void SysTechnologyViewer::Init() {
}

void SysTechnologyViewer::DoUI(int delta_time) {
    using common::components::science::TechnologicalProgress;
    // Display UI
    auto view = GetUniverse().view<common::components::Player>();
    entt::entity player = view.front();
    ImGui::Begin("Technology Information");
    if (GetUniverse().any_of<TechnologicalProgress>(player)) {
        auto& progress = GetUniverse().get<TechnologicalProgress>(player);
        for (entt::entity researched : progress.researched_techs) {
            ImGui::TextFmt("{}", gui::GetName(GetUniverse(), researched));
        }
    } else {
        ImGui::Text("Nope, no technology");
    }
    ImGui::End();
}

void SysTechnologyViewer::DoUpdate(int delta_time) {
}

void SysTechnologyProjectViewer::Init() {
}

void SysTechnologyProjectViewer::DoUI(int delta_time) {
    using common::components::science::ScientificResearch;
    // Display UI
    auto view = GetUniverse().view<common::components::Player>();
    entt::entity player = view.front();
    ImGui::Begin("Technology Research");
    if (GetUniverse().any_of<ScientificResearch>(player)) {
        auto& progress = GetUniverse().get<ScientificResearch>(player);
        for (auto& researched : progress.current_research) {
            ImGui::TextFmt("{} {}", gui::GetName(GetUniverse(), researched.first), researched.second);
        }
    } else {
        ImGui::Text("No Tech Research");
    }
    ImGui::End();
}

void SysTechnologyProjectViewer::DoUpdate(int delta_time) {
}
}  // namespace cqsp::client::systems
