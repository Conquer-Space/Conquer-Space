/*
 * Copyright 2021 Conquer Space
*/
#pragma once

#include "client/systems/sysgui.h"

namespace conquerspace {
namespace client {
namespace systems {
class SysCommand : public SysUserInterface {
 public:
    explicit SysCommand(conquerspace::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

    bool to_see = false;
 private:
    int selected_index = 0;
    entt::entity selected_planet = entt::null;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
