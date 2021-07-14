/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "client/systems/sysgui.h"
#include "common/components/event.h"

namespace conquerspace {
namespace client {
namespace systems {
namespace gui {
class SysEvent : public SysUserInterface {
 public:
    explicit SysEvent(conquerspace::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);
    void FireEvent();
    bool to_show;
};
}  // namespace gui
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
