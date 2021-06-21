/*
 * Copyright 2021 Conquer Space
*/
#pragma once

#include "client/systems/sysgui.h"

namespace conquerspace {
namespace client {
namespace systems {
class SysStarSystemTree : public SysUserInterface {
 public:
    explicit SysStarSystemTree(conquerspace::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    int selected_index = 0;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
