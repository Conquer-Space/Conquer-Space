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
#include "client/scenes/universe/interface/mapmodewindow.h"

#include "client/components/clientctx.h"
#include "mapmodewindow.h"

namespace cqsp::client::systems {
void MapModeWindow::Init() {}

void MapModeWindow::DoUI(int delta_time) {
    // Place on bottom right
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), 0, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    if (!ImGui::Begin("Map Mode", 0, ImGuiWindowFlags_NoTitleBar)) return;

    MapModeButton("No Map Mode", ctx::MapMode::NoMapMode);
    MapModeButton("Country Map Mode", ctx::MapMode::CountryMapMode);
    MapModeButton("Province Map Mode", ctx::MapMode::ProvinceMapMode);

    ImGui::End();
}

void MapModeWindow::SetMapMode(ctx::MapMode map_mode) {
    entt::entity current_map_mode = GetUniverse().view<ctx::MapMode>().front();
    GetUniverse().get<ctx::MapMode>(current_map_mode) = map_mode;
}

inline ctx::MapMode MapModeWindow::GetMapMode() {
    entt::entity current_map_mode = GetUniverse().view<ctx::MapMode>().front();
    return GetUniverse().get<ctx::MapMode>(current_map_mode);
}

void MapModeWindow::MapModeButton(const char* string, ctx::MapMode map_mode) {
    ctx::MapMode current_mode = GetMapMode();
    if (current_mode == map_mode) {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.8431372549, 0.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(181.f / 255.f, 148.f / 255.f, 16.f / 255.f, 1.f));
    }
    if (ImGui::Button(string)) {
        SetMapMode(map_mode);
    }
    if (current_mode == map_mode) {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
}

void MapModeWindow::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
