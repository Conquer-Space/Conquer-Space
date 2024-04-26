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
#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "client/systems/sysgui.h"

#define sysdebuggui_parameters                                                                                \
cqsp::engine::Application &app, common::Universe &universe,                                                   \
common::scripting::ScriptInterface &script_interface, const std::string_view &args,                           \
    CommandOutput &input

namespace cqsp::client::systems {
class SysDebugMenu : public SysUserInterface {
 public:
    explicit SysDebugMenu(cqsp::engine::Application &app);

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    void CqspMetricsWindow();
    void ShowWindows();
    void CreateMenuBar();
    void DrawConsole();
    void ConsoleInput();
    void DrawAssetWindow();

    bool to_show_window = false;
    bool to_show_metrics_window = false;
    bool to_show_imgui_about = false;
    bool to_show_implot_metrics = false;
    bool reclaim_focus = false;
    bool scroll_to_bottom = true;
    bool to_show_cqsp_metrics = false;
    bool to_show_asset_window = false;

    std::string command;
    std::string asset_search;
    std::vector<std::string> items;

    typedef std::vector<std::string> CommandOutput;
    typedef std::function<void(sysdebuggui_parameters)> DebugCommand_t;
    std::map<std::string, std::pair<std::string, DebugCommand_t>, std::less<>> commands;
    std::vector<ImVec2> fps_history;
    float fps_history_len = 10;

    std::map<std::string, std::vector<ImVec2>> history_maps;
};
}  // namespace cqsp::client::systems
