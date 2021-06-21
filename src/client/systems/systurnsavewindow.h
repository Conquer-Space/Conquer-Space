/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <vector>

#include "client/systems/sysgui.h"
#include "engine/application.h"

namespace conquerspace {
namespace client {
namespace systems {

class SysTurnSaveWindow : public SysUserInterface {
 public:
    explicit SysTurnSaveWindow(conquerspace::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

    void TogglePlayState();

 private:
    double last_tick = 0;
    bool to_tick = false;
    std::vector<int> tick_speeds{1000, 500, 333, 100, 50, 10, 1};
    int tick_speed = tick_speeds.size() / 2;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
