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
#include "sysdebuggui.h"

#include "GLFW/glfw3.h"
#include "client/components/clientctx.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/name.h"
#include "common/util/nameutil.h"
#include "common/util/profiler.h"
#include "glad/glad.h"

using cqsp::client::systems::SysDebugMenu;
using cqsp::engine::Application;

SysDebugMenu::SysDebugMenu(Application& app) : SysUserInterface(app) {
    using std::string_view;
    auto help_command = [&](sysdebuggui_parameters) {
        for (auto it = commands.begin(); it != commands.end(); it++) {
            input.push_back(fmt::format("{} - {}", it->first.c_str(), it->second.first));
        }
    };

    auto entity_command = [](sysdebuggui_parameters) {
        using cqsp::client::systems::MouseOverEntity;
        entt::entity ent = universe.view<MouseOverEntity>().front();
        if (ent == entt::null) {
            input.push_back(fmt::format("Mouse is over null"));
        } else {
            input.push_back(fmt::format("Mouse is over {}", ent));
        }
    };

    auto screen_clear = [](sysdebuggui_parameters) { input.clear(); };

    auto entitycount = [](sysdebuggui_parameters) {
        input.push_back(fmt::format("Total entities: {}, Alive: {}", universe.size(), universe.alive()));
    };

    auto entity_name = [](sysdebuggui_parameters) {
        if (std::all_of(args.begin(), args.end(), ::isdigit)) {
            namespace cqspc = cqsp::common::components;

            entt::entity entity = static_cast<entt::entity>(atoi(args.data()));
            std::string name = "N/A";
            if (universe.all_of<cqspc::Name>(entity)) {
                name = universe.get<cqspc::Name>(entity);
            }
            std::string identifier = "N/A";
            if (universe.all_of<cqspc::Identifier>(entity)) {
                identifier = universe.get<cqspc::Identifier>(entity);
            }
            input.push_back(fmt::format("Name: {}, {}", name, identifier));
        }
    };

    auto lua = [](sysdebuggui_parameters) { script_interface.RunScript(args); };

    commands = {{"help", {"Shows this help menu", help_command}},
                {"mouseon", {"Get the entitiy the mouse is over", entity_command}},
                {"clear", {"Clears screen", screen_clear}},
                {"entitycount", {"Gets number of entities", entitycount}},
                {"name", {"Gets name and identifier of entity", entity_name}},
                {"lua", {"Executes lua script", lua}}};
}

void SysDebugMenu::Init() {}

void SysDebugMenu::CqspMetricsWindow() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);
    ImGui::Begin("Conquer Space benchmarking", &to_show_cqsp_metrics);
    ImPlot::SetNextAxesLimits(GetApp().GetTime() - fps_history_len, GetApp().GetTime(), 0, 300, ImGuiCond_Always);
    if (ImPlot::BeginPlot("FPS", ImVec2(-1, 0), ImPlotFlags_NoChild)) {
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxis(ImAxis_Y1, "FPS", ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("FPS", &fps_history[0].x, &fps_history[0].y, fps_history.size(), 0, sizeof(float) * 2);
        ImPlot::EndPlot();
    }

    ImPlot::SetNextAxisLimits(ImAxis_X1, GetApp().GetTime() - fps_history_len, GetApp().GetTime(), ImGuiCond_Always);
    if (ImPlot::BeginPlot("Profiler", ImVec2(-1, 0), ImPlotFlags_NoMouseText | ImPlotFlags_NoChild)) {
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxis(ImAxis_Y1, "Run time (us)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupLegend(ImPlotLocation_SouthEast);
        for (auto it = history_maps.begin(); it != history_maps.end(); it++) {
            ImPlot::PlotLine(it->first.c_str(), &it->second[0].x, &it->second[0].y, it->second.size(), 0,
                             sizeof(float) * 2);
        }
        ImPlot::EndPlot();
    }
    profiler_information_map.clear();
    ImGui::End();
}

void cqsp::client::systems::SysDebugMenu::ShowWindows() {
    if (to_show_imgui_about) {
        ImGui::ShowAboutWindow(&to_show_imgui_about);
    }
    if (to_show_metrics_window) {
        ImGui::ShowMetricsWindow(&to_show_metrics_window);
    }
    if (to_show_implot_metrics) {
        ImPlot::ShowMetricsWindow(&to_show_implot_metrics);
    }
    if (to_show_cqsp_metrics) {
        CqspMetricsWindow();
    }
    if (to_show_asset_window) {
        DrawAssetWindow();
    }
}

void cqsp::client::systems::SysDebugMenu::CreateMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Benchmarks", 0, &to_show_cqsp_metrics);
            ImGui::MenuItem("Asset Debug Window", 0, &to_show_asset_window);

            if (ImGui::BeginMenu("ImGui")) {
                ImGui::MenuItem("About ImGui", 0, &to_show_imgui_about);
                ImGui::MenuItem("ImGui Debugger", 0, &to_show_metrics_window);
                ImGui::MenuItem("ImPlot Debugger", 0, &to_show_implot_metrics);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Extra UI Indicators")) {
            if (ImGui::MenuItem("Star System Viewer Debug")) {
                // FIXME(EhWhoamI)
                //auto& debug_info = GetApp().GetUniverse().ctx().emplace<ctx::StarSystemViewDebug>();
                //debug_info.to_show = !debug_info.to_show;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void cqsp::client::systems::SysDebugMenu::DrawConsole() {
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("consolearea", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    for (auto b : items) {
        bool color = (b[0] == '#');
        std::string str(b);
        int offset = 0;
        if (color) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.2f, 1.0f));
            offset = 1;
        }

        ImGui::TextUnformatted(b.substr(offset).c_str());
        if (color) {
            ImGui::PopStyleColor();
        }
    }
    if (scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
    }
    scroll_to_bottom = false;
    ImGui::EndChild();
}

void cqsp::client::systems::SysDebugMenu::ConsoleInput() {
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("DebugInput", &command,
                         ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion |
                             ImGuiInputTextFlags_CallbackHistory)) {
        std::string command_request = std::string(command);
        std::transform(command_request.begin(), command_request.end(), command_request.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!command_request.empty()) {
            bool no_command = true;
            for (auto it = commands.begin(); it != commands.end(); it++) {
                if (command_request.rfind(it->first, 0) != 0) {
                    continue;
                }
                it->second.second(GetApp(), GetUniverse(), GetScriptInterface(),
                                  command.length() == it->first.length() ? "" : command.substr(it->first.length() + 1),
                                  items);
                no_command = false;
                break;
            }
            if (no_command) {
                items.emplace_back("#Command does not exist!");
            }

            command = "";
            scroll_to_bottom = true;
        }
        reclaim_focus = true;
    }
    ImGui::PopItemWidth();

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1);
        reclaim_focus = false;
    }
}

void SysDebugMenu::DrawAssetWindow() {
    ImGui::Begin("Asset Debug Window", &to_show_asset_window);
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

void SysDebugMenu::DoUI(int delta_time) {
    ShowWindows();
    if (!to_show_window) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * .5f, ImGui::GetIO().DisplaySize.y * 0.75f),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("Debug Gui", &to_show_window, ImGuiWindowFlags_MenuBar | window_flags | ImGuiWindowFlags_NoDecoration);
    CreateMenuBar();
    DrawConsole();

    // Text and other things
    ImGui::Separator();
    ConsoleInput();

    ImGui::End();
}

void SysDebugMenu::DoUpdate(int delta_time) {
    if (!ImGui::GetIO().WantCaptureKeyboard && GetApp().ButtonIsReleased(GLFW_KEY_GRAVE_ACCENT)) {
        // Show window
        to_show_window = !to_show_window;
    }

    float time = GetApp().GetTime();
    float fps = GetApp().GetFps();
    // Update metrics
    if (!fps_history.empty() && (fps_history.begin()->x + fps_history_len) < time) {
        fps_history.erase(fps_history.begin());
    }
    fps_history.emplace_back(time, fps);

    for (auto it = profiler_information_map.begin(); it != profiler_information_map.end(); it++) {
        if (!history_maps[it->first].empty() && (history_maps[it->first].begin()->x + fps_history_len) < time) {
            history_maps[it->first].erase(history_maps[it->first].begin());
        }

        history_maps[it->first].emplace_back(time, it->second);
    }

    // Add lua logging information
    if (!GetScriptInterface().values.empty()) {
        // Fill up the things
        for (const auto& str : GetScriptInterface().values) {
            items.push_back("[lua] " + str);
        }
        GetScriptInterface().values.clear();
    }
}
