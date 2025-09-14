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
#include "client/scenes/objecteditor/assetwindow.h"

namespace cqsp::client::systems {   
void AssetWindow::Init() {

}
void AssetWindow::DoUI(int delta_time) {
    ImGui::Begin("Asset Debug Window");
    ImGui::Text("Search: ");
    ImGui::SameLine();
    ImGui::InputText("###asset_search", &asset_search);
    if (ImGui::BeginTable("asset_debug_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Accessed Times");
        ImGui::TableSetupColumn("Type");
        ImGui::TableHeadersRow();

        auto& asset_manager = GetApp().GetAssetManager();
        for (auto& package : asset_manager) {
            for (auto& asset : *package.second) {
                if (!asset_search.empty()) {
                    // Then look through the thing
                    std::string str = fmt::format("{}:{}", package.first, asset.first);
                    if (str.find(asset_search) == std::string::npos) {
                        continue;
                    }
                }
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextFmt("{}:{}", package.first, asset.first);
                ImGui::TableNextColumn();
                ImGui::TextFmt("{}", asset.second->accessed);
                ImGui::TableNextColumn();
                ImGui::TextFmt("{}", cqsp::asset::ToString(asset.second->GetAssetType()));
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}
void AssetWindow::DoUpdate(int delta_time) {

}
};  // namespace cqsp::client::systems
