/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/sysdebuggui.h"

#include "client/systems/sysstarsystemrenderer.h"
#include "common/util/profiler.h"

conquerspace::client::systems::SysDebugMenu::SysDebugMenu(
    conquerspace::engine::Application& app) : SysUserInterface(app) {
    auto help_command = [&](conquerspace::engine::Application& app,
                                const std::string_view& args, std::vector<std::string>& input) {
        for (auto it = commands.begin(); it != commands.end(); it++) {
            input.push_back(fmt::format("{} - {}", it->first.c_str(), it->second.first));
        }
    };

    auto entity_command = [](conquerspace::engine::Application& app,
                             const std::string_view& args,
                             std::vector<std::string>& input) {
        entt::entity ent =
            app.GetUniverse().view<conquerspace::client::systems::MouseOverEntity>().front();
        if (ent == entt::null) {
            input.push_back(fmt::format("Mouse is over null"));
        } else {
            input.push_back(fmt::format("Mouse is over {}", ent));
        }
    };

    auto screen_clear = [](conquerspace::engine::Application& app,
                             const std::string_view& args,
                             std::vector<std::string>& input) {
        input.clear();
    };

    auto entitycount = [](conquerspace::engine::Application& app,
                             const std::string_view& args,
                             std::vector<std::string>& input) {
        input.push_back(fmt::format("Total entities: {}, Alive: {}",
                        app.GetUniverse().size(), app.GetUniverse().alive()));
    };

    commands = {
        {"help", {"Shows this help menu", help_command}},
        {"mouseon", {"Get the entitiy the mouse is over", entity_command}},
        {"clear", {"Clears screen", screen_clear}},
        {"entitycount", {"Gets number of entities", entitycount}}
    };
}

void conquerspace::client::systems::SysDebugMenu::Init() {}

void conquerspace::client::systems::SysDebugMenu::DoUI(int delta_time) {
    if (!to_show_window) {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Appearing);
    ImGui::Begin("Debug Gui", &to_show_window, ImGuiWindowFlags_MenuBar | window_flags);
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

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y +
                                                        ImGui::GetFrameHeightWithSpacing();
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
    if (ImGui::InputText("Label", &command, ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory)) {
        std::transform(command.begin(), command.end(), command.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        if (!command.empty()) {
            bool no_command = true;
            for (auto it = commands.begin(); it != commands.end(); it++) {
                if (command.rfind(it->first, 0) == 0) {
                    it->second.second(GetApp(), it->first, items);
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

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1);
        reclaim_focus = false;
    }

    ImGui::End();

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
        if (ImPlot::BeginPlot("Profiler", "ms", "Run time (s)", ImVec2(-1, 0),
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
}

void conquerspace::client::systems::SysDebugMenu::DoUpdate(int delta_time) {
    if (!ImGui::GetIO().WantCaptureKeyboard && GetApp().ButtonIsReleased(GLFW_KEY_GRAVE_ACCENT)) {
        // Show window
        to_show_window = !to_show_window;
    }

    // Update metrics
    if (!fps_history.empty() &&
                (fps_history.begin()->x + fps_history_len) < GetApp().GetTime()) {
        fps_history.erase(fps_history.begin());
    }
    fps_history.push_back(ImVec2(static_cast<float>(GetApp().GetTime()), GetApp().GetFps()));

    for (auto it = profiler_information_map.begin(); it != profiler_information_map.end(); it++) {
        if (!history_maps[it->first].empty() &&
            (history_maps[it->first].begin()->x + fps_history_len) <
                static_cast<float>(GetApp().GetTime())) {
            history_maps[it->first].erase(history_maps[it->first].begin());
        }

        history_maps[it->first].push_back(ImVec2(static_cast<float>(GetApp().GetTime()),
                                                                static_cast<float>(it->second)));
    }
}
