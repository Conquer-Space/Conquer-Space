/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <string>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>

#include "client/systems/sysgui.h"

namespace conquerspace {
namespace client {
namespace systems {
typedef std::function<void(conquerspace::engine::Application& app,
                           const std::string_view& args,
                           std::vector<std::string>& input)>
    DebugCommand_t;
class SysDebugMenu : public SysUserInterface {
 public:
    explicit SysDebugMenu(conquerspace::engine::Application& app);

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    bool to_show_window = false;
    bool to_show_metrics_window = false;
    bool to_show_imgui_about = false;
    bool to_show_implot_metrics = false;
    bool reclaim_focus = false;
    bool scroll_to_bottom = true;
    bool to_show_cqsp_metrics = false;
    std::string command;
    std::vector<std::string> items;
    std::map<std::string, std::pair<std::string, DebugCommand_t>, std::less<>> commands;
    std::vector<ImVec2> fps_history;
    float fps_history_len = 10;

    std::map<std::string, std::vector<ImVec2>> history_maps;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
