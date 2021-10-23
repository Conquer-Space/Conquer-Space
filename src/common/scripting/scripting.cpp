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
#include "common/scripting/scripting.h"

#include <fmt/format.h>

#include <string>
#include <iostream>
#include <memory>

#include "common/util/logging.h"

using cqsp::scripting::ScriptInterface;

ScriptInterface::ScriptInterface() {
    open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package);
    // Initialize loggers
    logger = cqsp::common::util::make_logger("lua");
    // Add a sink to get the scripting log
    ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128);
    ringbuffer_sink->set_pattern("%v");
    logger->sinks().push_back(ringbuffer_sink);
}

void ScriptInterface::ParseResult(const sol::protected_function_result& result) {
    if (!result.valid()) {
        sol::error err = result;
        std::string what = err.what();
        values.push_back(fmt::format("{}", what));
        SPDLOG_LOGGER_INFO(logger, "{}", what);
    }
}

void ScriptInterface::RunScript(std::string_view str) {
    ParseResult(safe_script(str));
}

void ScriptInterface::RegisterDataGroup(std::string_view name) {
    script(fmt::format(R"({} = {{
        data = {{}},
        len = 0,
        insert = function(self, info)
            table.insert(self.data, info)
            self.len = self.len + 1
        end
        }}
    )", name).c_str());
}

void ScriptInterface::Init() {
    // Set print functions
    set_function("print", sol::overload(
        [&] (const char * y) {
            SPDLOG_LOGGER_INFO(logger, "{}", y);
        },
        [&](int y) {
            SPDLOG_LOGGER_INFO(logger, "{}", y);
        },
        [&](double y) {
            SPDLOG_LOGGER_INFO(logger, "{}", y);
    }));
}

int ScriptInterface::GetLength(std::string_view a) {
    return static_cast<int>((*this)[a]["len"]);
}

std::vector<std::string> cqsp::scripting::ScriptInterface::GetLogs() {
    return ringbuffer_sink->last_formatted();
}
