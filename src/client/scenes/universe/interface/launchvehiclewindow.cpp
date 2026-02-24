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
#include "client/scenes/universe/interface/launchvehiclewindow.h"

#include "client/components/clientctx.h"
#include "core/components/launchvehicle.h"
#include "core/components/name.h"
#include "core/components/player.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems {
void LaunchVehicleWindow::Init() {
    search_text.fill(0);
    design.vehicle_name.fill(0);
}

void LaunchVehicleWindow::DoUI(int delta_time) {
    // Get player
    entt::entity player = GetUniverse().view<core::components::Player>().front();
    if (!GetUniverse().all_of<core::components::SpaceCapability>(player)) {
        return;
    }

    core::components::SpaceCapability& capability = GetUniverse().get<core::components::SpaceCapability>(player);
    bool selected = GetUniverse().ctx().at<ctx::SelectedMenu>() == ctx::SelectedMenu::SpaceMenu;
    if (!selected) {
        return;
    }
    ImGui::Begin("Launch Vehicles", &selected);
    // Now display our launch vehicles and types as well and so on
    ImGui::TextFmt("Launch Vehicles: {}", capability.launch_vehicle_list.size());
    ImGui::SameLine();
    if (ImGui::Button("+###add_launch_vehicle")) {
        is_creating_launch_vehicle = true;
    }
    ImGui::BeginChild("launch_vehicle_viewer_left", ImVec2(300, 700));
    ImGui::InputText("##launch_vehicle_viewer_search_text", search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild("launch_vehicle_viewer_scroll");
    for (entt::entity launch_vehicle : capability.launch_vehicle_list) {
        bool is_selected = launch_vehicle == selected_launch_vehicle;
        std::string name = core::util::GetName(GetUniverse(), launch_vehicle);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        // Now check if the string is in stuff
        if (ImGui::SelectableFmt("{}", &is_selected, name)) {
            selected_launch_vehicle = launch_vehicle;
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("launch_vehicle_viewer_right", ImVec2(400, 700));
    LaunchVehicleWindowRight();
    ImGui::EndChild();
    ImGui::End();

    CreateLaunchVehicleWindow();

    if (!selected) {
        GetUniverse().ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::NoMenu;
    }
}

void LaunchVehicleWindow::CreateLaunchVehicleWindow() {
    if (!is_creating_launch_vehicle) {
        return;
    }
    ImGui::Begin("Create Launch Vehicle", &is_creating_launch_vehicle);
    // Now we design one or something
    // then we have a design project but let's just instantly do that for now
    ImGui::Text("Name:");
    ImGui::SameLine();
    ImGui::InputText("###Name", design.vehicle_name.data(), design.vehicle_name.size());
    ImGui::DragFloat("Reliability", &design.reliability, 1.f, 0.f, 100.f);
    ImGui::DragFloat("Fairing Size", &design.fairing_size, 1.f, 0.f);
    ImGui::DragFloat("Mass to Orbit", &design.mass_to_orbit, 1.f, 0.f);
    if (ImGui::Button("Design!")) {
        // Create launch vehicle
        entt::entity player = GetUniverse().GetPlayer();
        core::components::SpaceCapability& capability = GetUniverse().get<core::components::SpaceCapability>(player);
        entt::entity new_launch_vehicle = GetUniverse().create();
        auto& launch_vehicle = GetUniverse().emplace<core::components::LaunchVehicle>(new_launch_vehicle);
        launch_vehicle.fairing_size = static_cast<double>(design.fairing_size / 100.f);
        launch_vehicle.reliability = static_cast<double>(design.reliability);
        launch_vehicle.mass_to_orbit = static_cast<double>(design.mass_to_orbit);
        launch_vehicle.manufacture_time = design.manufacture_time;
        GetUniverse().emplace<core::components::Name>(new_launch_vehicle, std::string(design.vehicle_name.data()));
        capability.launch_vehicle_list.push_back(new_launch_vehicle);
        is_creating_launch_vehicle = false;
    }
    ImGui::End();
}

void LaunchVehicleWindow::LaunchVehicleWindowRight() {
    if (!GetUniverse().valid(selected_launch_vehicle)) {
        return;
    }
    std::string name = core::util::GetName(GetUniverse(), selected_launch_vehicle);
    auto& launch_vehicle = GetUniverse().get<core::components::LaunchVehicle>(selected_launch_vehicle);
    ImGui::TextFmt("{}", name);
    ImGui::Separator();
    ImGui::TextFmt("Reliability: {}%", launch_vehicle.reliability * 100);
    ImGui::TextFmt("Mass to Orbit: {} tons", launch_vehicle.mass_to_orbit);
    ImGui::TextFmt("Faring Size: {} m3", launch_vehicle.fairing_size);
}

void LaunchVehicleWindow::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
