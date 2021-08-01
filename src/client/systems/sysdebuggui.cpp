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
#include "client/systems/sysdebuggui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "client/systems/sysstarsystemrenderer.h"
#include "common/components/name.h"
#include "common/util/profiler.h"

using conquerspace::client::systems::SysDebugMenu;
using conquerspace::engine::Application;

SysDebugMenu::SysDebugMenu(Application& app) : SysUserInterface(app) {
    using std::string_view;
    auto help_command = [&](Application& app, const string_view& args, CommandOutput& input) {
        for (auto it = commands.begin(); it != commands.end(); it++) {
            input.push_back(fmt::format("{} - {}", it->first.c_str(), it->second.first));
        }
    };

    auto entity_command = [](Application& app, const string_view& args, CommandOutput& input) {
        using conquerspace::client::systems::MouseOverEntity;
        entt::entity ent = app.GetUniverse().view<MouseOverEntity>().front();
        if (ent == entt::null) {
            input.push_back(fmt::format("Mouse is over null"));
        } else {
            input.push_back(fmt::format("Mouse is over {}", ent));
        }
    };

    auto screen_clear = [](Application& app, const string_view& args, CommandOutput& input) {
        input.clear();
    };

    auto entitycount = [](Application& app, const string_view& args, CommandOutput& input) {
        input.push_back(fmt::format("Total entities: {}, Alive: {}",
                        app.GetUniverse().size(), app.GetUniverse().alive()));
    };

    auto entity_name = [](Application& app, const string_view& args, CommandOutput& input) {
        if (std::all_of(args.begin(), args.end(), ::isdigit)) {
            entt::entity entity = static_cast<entt::entity>(atoi(args.data()));
            std::string name = "N/A";
            if (app.GetUniverse().all_of<conquerspace::common::components::Name>(entity)) {
                name = app.GetUniverse().get<conquerspace::common::components::Name>(entity).name;
            }
            std::string identifier = "N/A";
            if (app.GetUniverse().all_of<conquerspace::common::components::Identifier>(entity)) {
                identifier = app.GetUniverse().get<conquerspace::common::components::Identifier>(entity).identifier;
            }
            input.push_back(fmt::format("Name: {}, {}", name, identifier));
        }
    };

    auto lua = [](Application& app, const string_view& args, CommandOutput& input) {
        app.GetScriptInterface().RunScript(args);
    };


    commands = {
        {"help", {"Shows this help menu", help_command}},
        {"mouseon", {"Get the entitiy the mouse is over", entity_command}},
        {"clear", {"Clears screen", screen_clear}},
        {"entitycount", {"Gets number of entities", entitycount}},
        {"name", {"Gets name and identifier of entity", entity_name}},
        {"lua", {"Executes lua script", lua}
}
    };
}

void SysDebugMenu::Init() {}

void SysDebugMenu::CqspMetricsWindow() {
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);
        ImGui::Begin("Conquer Space benchmarking", &to_show_cqsp_metrics);
        ImPlot::SetNextPlotLimitsX(GetApp().GetTime() - fps_history_len,
                                            GetApp().GetTime(), ImGuiCond_Always);
        ImPlot::SetNextPlotLimitsY(0, 300, ImGuiCond_Always);
        if (ImPlot::BeginPlot("FPS", "Time (s)", "FPS", ImVec2(-1, 0), ImPlotFlags_NoChild,
                                            ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
            ImPlot::PlotLine("FPS", &fps_history[0].x, &fps_history[0].y,
                             fps_history.size(), 0, sizeof(float) * 2);
            ImPlot::EndPlot();
        }

        ImPlot::SetNextPlotLimitsX(GetApp().GetTime() - fps_history_len, GetApp().GetTime(),
                                                                            ImGuiCond_Always);
        if (ImPlot::BeginPlot("Profiler", "Time (s)", "Run time (us)", ImVec2(-1, 0),
                                    ImPlotFlags_NoMousePos | ImPlotFlags_NoChild,
                                    ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
            ImPlot::SetLegendLocation(ImPlotLocation_SouthEast);
            for (auto it = history_maps.begin(); it != history_maps.end();
                 it++) {
                ImPlot::PlotLine(it->first.c_str(), &it->second[0].x, &it->second[0].y,
                             it->second.size(), 0, sizeof(float) * 2);
            }
            ImPlot::EndPlot();
        }
        profiler_information_map.clear();
        ImGui::End();
}

void SysDebugMenu::DoUI(int delta_time) {
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

    if (!to_show_window) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x*.5f,
                                ImGui::GetIO().DisplaySize.y * 0.75f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("Debug Gui", &to_show_window,
                 ImGuiWindowFlags_MenuBar | window_flags |
                     ImGuiWindowFlags_NoDecoration);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Benckmarks", 0, &to_show_cqsp_metrics);
            if (ImGui::BeginMenu("ImGui")) {
                ImGui::MenuItem("About ImGui", 0, &to_show_imgui_about);
                ImGui::MenuItem("ImGui Debugger", 0, &to_show_metrics_window);
                ImGui::MenuItem("ImPlot Debugger", 0, &to_show_implot_metrics);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    const float footer_height_to_reserve =
        ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
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

    // Text and other things
    ImGui::Separator();
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("DebugInput", &command, ImGuiInputTextFlags_EnterReturnsTrue |
                             ImGuiInputTextFlags_CallbackCompletion |
                             ImGuiInputTextFlags_CallbackHistory)) {
        std::string command_request = std::string(command);
        std::transform(command_request.begin(), command_request.end(), command_request.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        if (!command_request.empty()) {
            bool no_command = true;
            for (auto it = commands.begin(); it != commands.end(); it++) {
                if (command_request.rfind(it->first, 0) == 0) {
                    it->second.second(GetApp(), command.length()==it->first.length() ? "" : command.substr(it->first.length()+1) , items);
                    no_command = false;
                    break;
                }
            }
            if (no_command) {
                items.push_back("#Command does not exist!");
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
    fps_history.push_back(ImVec2(time, fps));

    for (auto it = profiler_information_map.begin(); it != profiler_information_map.end(); it++) {
        if (!history_maps[it->first].empty() &&
            (history_maps[it->first].begin()->x + fps_history_len) < time) {
            history_maps[it->first].erase(history_maps[it->first].begin());
        }

        history_maps[it->first].push_back(ImVec2(time, it->second));
    }

    // Add lua logging information
    if (!GetApp().GetScriptInterface().values.empty()) {
        // Fill up the things
        for (auto str : GetApp().GetScriptInterface().values) {
            items.push_back("[lua] " + str);
        }
        GetApp().GetScriptInterface().values.clear();
    }
}
