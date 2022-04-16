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
#include "client/systems/sysfieldviewer.h"

#include "common/components/science.h"
#include "common/components/name.h"

void cqsp::client::systems::SysFieldViewer::Init() {}

void cqsp::client::systems::SysFieldViewer::DoUI(int delta_time) {
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
    ImGui::Begin("Field Viewer");
    // Loop through fields
    const int height = 300;
    ImGui::BeginChild("Field List", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto view = GetUniverse().view<common::components::science::Field>();
    for (entt::entity entity : view) {
        if (ImGui::Selectable(GetUniverse()
                                  .get<common::components::Name>(entity)
                                  .name.c_str(),
                              selected_tech == entity)) {
            selected_tech = entity;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("FieldInformation", ImVec2(-1, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    FieldInformationWindow();
    ImGui::EndChild();
    ImGui::End();
}

void cqsp::client::systems::SysFieldViewer::DoUpdate(int delta_time) {}

void cqsp::client::systems::SysFieldViewer::FieldInformationWindow() {
    if (!GetUniverse().valid(selected_tech)) {
        return;
    }
    if (!GetUniverse().any_of<common::components::science::Field>(selected_tech)) {
        return;
    }
    auto& field = GetUniverse().get<common::components::science::Field>(selected_tech);
    auto& name = GetUniverse().get<common::components::Name>(selected_tech);
    ImGui::Text(name.name.c_str());
    if (GetUniverse().any_of<common::components::Description>(selected_tech)) {
        auto& desc = GetUniverse().get<common::components::Description>(selected_tech);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.7, 1));
        ImGui::TextWrapped(desc.description.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
    ImGui::Text("Parents");
    for (auto& f : field.parents) {
        ImGui::Text(GetUniverse().get<common::components::Name>(f).name.c_str());
    }
    ImGui::Separator();
    ImGui::Text("Adjacents");

    for (auto& f : field.adjacent) {
        ImGui::Text(GetUniverse().get<common::components::Name>(f).name.c_str());
    }
}
