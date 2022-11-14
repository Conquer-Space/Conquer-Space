/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "common/systems/scriptrunner.h"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "common/util/profiler.h"

cqsp::common::systems::SysScript::SysScript(Game &game) : ISimulationSystem(game) {
    sol::optional<std::vector<sol::table>> optional = game.GetScriptInterface()["events"]["data"];
    events = *optional;
}

cqsp::common::systems::SysScript::~SysScript() {
    // So it doesn't crash when we delete this
    for (auto &evet : events) {
        evet.abandon();
    }
    events.clear();
}

void cqsp::common::systems::SysScript::DoSystem() {
    BEGIN_TIMED_BLOCK(ScriptEngine);
    GetGame().GetScriptInterface()["date"] = GetUniverse().date.GetDate();
    for (auto &a : events) {
        sol::protected_function_result result = a["on_tick"](a);
        GetGame().GetScriptInterface().ParseResult(result);
    }
    END_TIMED_BLOCK(ScriptEngine);
}
