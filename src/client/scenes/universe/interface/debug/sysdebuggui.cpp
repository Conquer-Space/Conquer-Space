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
#include "client/scenes/universe/interface/debug/sysdebuggui.h"

#include <RmlUi/Debugger.h>

#include <filesystem>

#include "GLFW/glfw3.h"
#include "client/components/clientctx.h"
#include "client/scenes/universe/views/starsystemrenderer.h"
#include "core/components/name.h"
#include "core/systems/history/sysmarketdumper.h"
#include "core/util/nameutil.h"
#include "core/util/profiler.h"
#include "glad/glad.h"

namespace cqsp::client::systems {
using engine::Application;
namespace components = core::components;

SysDebugMenu::SysDebugMenu(Application& app) : SysUserInterface(app), asset_window(app) {
    asset_window.SetWindowOpenPtr(&to_show_asset_window);
    using std::string_view;
    auto help_command = [&](sysdebuggui_parameters) {
        for (auto it = commands.begin(); it != commands.end(); it++) {
            input.push_back(fmt::format("{} - {}", it->first.c_str(), it->second.first));
        }
    };

    auto entity_command = [](sysdebuggui_parameters) {
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
            entt::entity entity = static_cast<entt::entity>(atoi(args.data()));
            std::string name = "N/A";
            if (universe.all_of<components::Name>(entity)) {
                name = universe.get<components::Name>(entity);
            }
            std::string identifier = "N/A";
            if (universe.all_of<components::Identifier>(entity)) {
                identifier = universe.get<components::Identifier>(entity);
            }
            input.push_back(fmt::format("Name: {}, {}", name, identifier));
        }
    };

    auto log_market = [](sysdebuggui_parameters) {
        namespace fs = std::filesystem;
        std::string output_dir_name = "output";
        if (!fs::exists(output_dir_name)) {
            fs::create_directory(output_dir_name);
        }
        core::systems::history::SaveUniverseMarketState(universe, output_dir_name + "/market_log.hjson");
        input.emplace_back("Wrote market state!");
    };

    auto lua = [](sysdebuggui_parameters) { script_interface.RunScript(args); };

    commands = {{"help", {"Shows this help menu", help_command}},
                {"mouseon", {"Get the entitiy the mouse is over", entity_command}},
                {"clear", {"Clears screen", screen_clear}},
                {"entitycount", {"Gets number of entities", entitycount}},
                {"name", {"Gets name and identifier of entity", entity_name}},
                {"lua", {"Executes lua script", lua}},
                {"log_market", {"Logs market into a csv file", log_market}}};
    to_show_rmlui_window = Rml::Debugger::IsVisible();
    GetUniverse().ctx().emplace<ctx::StarSystemViewDebug>();
}

void SysDebugMenu::Init() {}

void SysDebugMenu::CqspMetricsWindow() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);
    ImGui::Begin("Conquer Space benchmarking", &to_show_cqsp_metrics);
    ImPlot::SetNextAxesLimits(GetApp().GetTime() - fps_history_len, GetApp().GetTime(), 0, 300, ImGuiCond_Always);
    if (ImPlot::BeginPlot("FPS", ImVec2(-1, 0), ImPlotFlags_NoChild)) {
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxis(ImAxis_Y1, "FPS", ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("FPS", time_history.data(), fps_history.data(), fps_history.size(), 0, 0,
                         sizeof(std::decay_t<decltype(*time_history.begin())>));
        ImPlot::EndPlot();
    }

    ImPlot::SetNextAxisLimits(ImAxis_X1, GetApp().GetTime() - fps_history_len, GetApp().GetTime(), ImGuiCond_Always);
    if (ImPlot::BeginPlot("Profiler", ImVec2(-1, 0), ImPlotFlags_NoMouseText | ImPlotFlags_NoChild)) {
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxis(ImAxis_Y1, "Run time (us)", ImPlotAxisFlags_AutoFit);
        ImPlot::SetupLegend(ImPlotLocation_SouthEast);
        for (auto it = history_maps.begin(); it != history_maps.end(); it++) {
            ImPlot::PlotLine(it->first.c_str(), &it->second[0].x, &it->second[0].y, it->second.size(), 0, 0,
                             sizeof(float) * 2);
        }
        ImPlot::EndPlot();
    }
    get_profile_information().clear();
    ImGui::End();
}

void SysDebugMenu::ShowWindows(double delta_time) {
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
        asset_window.DoUI(delta_time);
    }
}

void SysDebugMenu::CreateMenuBar() {
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
            if (ImGui::BeginMenu("RmlUi")) {
                if (ImGui::MenuItem("RmlUi Debugger", 0, &to_show_rmlui_window)) {
                    Rml::Debugger::SetVisible(to_show_rmlui_window);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Extra UI Indicators")) {
            if (ImGui::MenuItem("Star System Viewer Debug")) {
                auto& debug_info = GetUniverse().ctx().at<ctx::StarSystemViewDebug>();
                debug_info.to_show = !debug_info.to_show;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void SysDebugMenu::DrawConsole() {
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("consolearea", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    for (auto b : items) {
        bool color = (b[0] == '#');
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

void SysDebugMenu::ConsoleInput() {
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
                if (!command_request.starts_with(it->first)) {
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

void SysDebugMenu::DoUI(int delta_time) {
    ShowWindows(delta_time);
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
    if (!time_history.empty() && (*(time_history.begin()) + fps_history_len) < time) {
        fps_history.erase(fps_history.begin());
        time_history.erase(time_history.begin());
    }
    time_history.emplace_back(time);
    fps_history.emplace_back(fps);

    for (auto it = get_profile_information().begin(); it != get_profile_information().end(); it++) {
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

}  // namespace cqsp::client::systems