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
#include "sysoptionswindow.h"

#include "common/version.h"
#include "engine/cqspgui.h"

namespace cqsp::client::systems {
void ShowOptionsWindow(bool* open, cqsp::engine::Application& app) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.8f),
                             ImGuiCond_Always);
    ImGui::Begin("Options", open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::BeginTabBar("settingstabs")) {
        if (ImGui::BeginTabItem("Graphics")) {
            const ImVec2 common_resolutions[] = {
                ImVec2(1024, 768),  ImVec2(1280, 1024), ImVec2(1280, 720),  ImVec2(1280, 800),  ImVec2(1360, 768),
                ImVec2(1366, 768),  ImVec2(1440, 900),  ImVec2(1600, 900),  ImVec2(1680, 1050), ImVec2(1920, 1200),
                ImVec2(1920, 1080), ImVec2(2560, 1440), ImVec2(2560, 1080), ImVec2(3440, 1440), ImVec2(3840, 2160)};
            static int item_current_idx = 0;
            for (int n = 0; n < IM_ARRAYSIZE(common_resolutions); n++) {
                if (app.GetClientOptions().GetOptions()["window"]["width"] ==
                        static_cast<int>(common_resolutions[n].x) &&
                    app.GetClientOptions().GetOptions()["window"]["height"] ==
                        static_cast<int>(common_resolutions[n].y)) {
                    item_current_idx = n;
                }
            }

            ImGui::Text("Window Dimensions");
            ImGui::SameLine();
            if (ImGui::BeginCombo(
                    "##window size combo box",
                    fmt::format("{}x{}", common_resolutions[item_current_idx].x, common_resolutions[item_current_idx].y)
                        .c_str())) {
                for (int n = 0; n < IM_ARRAYSIZE(common_resolutions); n++) {
                    const bool is_selected = (item_current_idx == n);
                    if (CQSPGui::DefaultSelectable(
                            fmt::format("{}x{}", common_resolutions[n].x, common_resolutions[n].y).c_str(),
                            is_selected)) {
                        item_current_idx = n;

                        // Change options
                        app.GetClientOptions().GetOptions()["window"]["width"] =
                            static_cast<int>(common_resolutions[n].x);
                        app.GetClientOptions().GetOptions()["window"]["height"] =
                            static_cast<int>(common_resolutions[n].y);

                        if (!static_cast<bool>(app.GetClientOptions().GetOptions()["full_screen"])) {
                            // Set resolution
                            app.SetWindowDimensions(common_resolutions[n].x, common_resolutions[n].y);
                        }
                    }

                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            static bool full_screen_checkbox = static_cast<bool>(app.GetClientOptions().GetOptions()["full_screen"]);
            ImGui::Text("Full Screen");
            ImGui::SameLine();
            if (CQSPGui::DefaultCheckbox("##Full Screen", &full_screen_checkbox)) {
                // Switch screen
                app.SetFullScreen(full_screen_checkbox);
                app.GetClientOptions().GetOptions()["full_screen"] = full_screen_checkbox;
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sound")) {
            static float volume = app.GetClientOptions().GetOptions()["audio"]["music"];

            ImGui::Text("Music Volume");
            ImGui::SameLine();
            if (CQSPGui::SliderFloat("###Volume", &volume, 0, 1, "")) {
                // Then set volume
                app.GetAudioInterface().SetMusicVolume(volume);
                app.GetClientOptions().GetOptions()["audio"]["music"] = volume;
            }

            static float ui_volume = app.GetClientOptions().GetOptions()["audio"]["ui"];
            ImGui::Text("UI Volume");
            ImGui::SameLine();
            if (CQSPGui::SliderFloat("###UiVolume", &ui_volume, 0, 1, "")) {
                // Then set volume
                app.GetAudioInterface().SetChannelVolume(1, ui_volume);
                app.GetClientOptions().GetOptions()["audio"]["ui"] = ui_volume;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
}  // namespace cqsp::client::systems
