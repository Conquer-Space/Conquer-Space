/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "engine/application.h"

namespace conquerspace {
namespace client {
namespace systems {
class SysUserInterface {
 public:
    explicit SysUserInterface(conquerspace::engine::Application &);
    virtual void Init() = 0;
    virtual void DoUI(int delta_time) = 0;
    virtual void DoUpdate(int delta_time) = 0;

    conquerspace::engine::Application &GetApp() { return m_app; }

 private:
    conquerspace::engine::Application &m_app;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
