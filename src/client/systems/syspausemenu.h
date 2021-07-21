/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "client/systems/sysgui.h"

namespace conquerspace {
namespace client {
namespace systems {
class SysPauseMenu : public SysUserInterface {
 public:
    explicit SysPauseMenu(conquerspace::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    bool to_show = false;
    bool to_show_options_window = false;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
