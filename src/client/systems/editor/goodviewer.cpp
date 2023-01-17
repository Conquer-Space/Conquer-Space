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
#include "client/systems/editor/goodviewer.h"

#include "client/systems/gui/systooltips.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/nameutil.h"

void cqsp::client::systems::SysGoodViewer::Init() {}

void cqsp::client::systems::SysGoodViewer::DoUI(int delta_time) {
    ImGui::Begin("Good Viewer");
    // List out all the stuff
    auto goods = GetUniverse().view<common::components::Good>();
    ImGui::TextFmt("Goods: {}", goods.size());
    ImGui::BeginChild("Good_viewer_left", ImVec2(300, -1));
    for (entt::entity good : goods) {
        bool is_selected = good == selected_good;
        if (ImGui::SelectableFmt("{}", &is_selected, common::util::GetName(GetUniverse(), good))) {
            selected_good = good;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("good_viewer_right", ImVec2(300, -1));
    GoodViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void cqsp::client::systems::SysGoodViewer::DoUpdate(int delta_time) {}

void cqsp::client::systems::SysGoodViewer::GoodViewerRight() {
    namespace cc = common::components;
    if (selected_good == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }
    ImGui::TextFmt("Name: {}", common::util::GetName(GetUniverse(), selected_good));
    ImGui::TextFmt("Identifier: {}", GetUniverse().get<cc::Identifier>(selected_good).identifier);
    if (GetUniverse().any_of<cc::Matter>(selected_good)) {
        auto& good_comp = GetUniverse().get<cc::Matter>(selected_good);
        ImGui::TextFmt("Mass: {} kg", good_comp.mass);
        ImGui::TextFmt("Volume: {} m3", good_comp.volume);
    }
    ImGui::Separator();
    ImGui::TextFmt("Tags");
    ImGui::BeginChild("Tags_a");
    if (GetUniverse().any_of<cc::Mineral>(selected_good)) {
        ImGui::TextFmt("mineral");
    }
    if (GetUniverse().any_of<cc::CapitalGood>(selected_good)) {
        ImGui::TextFmt("capitalgood");
    }
    ImGui::Separator();
    if (GetUniverse().any_of<cc::Price>(selected_good)) {
        auto& price_comp = GetUniverse().get<cc::Price>(selected_good);
        ImGui::TextFmt("Initial Price: {}", price_comp.price);
    }
    ImGui::EndChild();
}
