/*
 * Copyright 2021 Conquer Space
 */
#include "common/systems/scriptrunner.h"

#include <spdlog/spdlog.h>

#include <vector>
#include <string>

conquerspace::common::systems::SysEventScriptRunner::SysEventScriptRunner(
    conquerspace::common::components::Universe &_universe,
    scripting::ScriptInterface &interface) : universe(_universe), m_script_interface(interface) {
        sol::optional<std::vector<sol::table>> optional = m_script_interface["events"]["data"];
        events = *optional;
    // Add functions and stuff
}

void conquerspace::common::systems::SysEventScriptRunner::ScriptEngine() {
    m_script_interface["date"] = universe.date.GetDate();
    for (auto &a : events) {
        sol::protected_function_result result = a["on_tick"](a);
        if (!result.valid()) {
            sol::error err = result;
            std::string what = err.what();
            SPDLOG_INFO("{}", what);
        }
    }
}

conquerspace::common::systems::SysEventScriptRunner::~SysEventScriptRunner() {
    // So it doesn't crash when we delete this
    for (auto& evet : events) {
        evet.abandon();
    }
    events.clear();
}
