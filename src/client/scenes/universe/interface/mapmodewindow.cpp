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
    if (ImGui::Button("No Map Mode")) {
        SetMapMode(ctx::MapMode::NoMapMode);
    }
    if (ImGui::Button("Country Map Mode")) {
        SetMapMode(ctx::MapMode::CountryMapMode);
    }
    if (ImGui::Button("Province Map Mode")) {
        SetMapMode(ctx::MapMode::ProvinceMapMode);
    }
    ImGui::End();
}

void MapModeWindow::SetMapMode(ctx::MapMode map_mode) {
    entt::entity current_map_mode = GetUniverse().view<ctx::MapMode>().front();
    GetUniverse().get<ctx::MapMode>(current_map_mode) = map_mode;
}

void MapModeWindow::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
