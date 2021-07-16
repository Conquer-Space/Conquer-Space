/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>

#include "common/scripting/scripting.h"
#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
class SysEventScriptRunner {
 public:
    SysEventScriptRunner(conquerspace::common::components::Universe& _universe,
                         scripting::ScriptInterface& interface);
    void ScriptEngine();
    ~SysEventScriptRunner();
 private:
    scripting::ScriptInterface &m_script_interface;
    conquerspace::common::components::Universe& universe;
    std::vector<sol::table> events;
};
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
