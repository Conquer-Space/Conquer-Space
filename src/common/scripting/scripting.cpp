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


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/format.h>

#include <string>
#include <iostream>

auto lua_logger = spdlog::stdout_color_mt("lua");

using cqsp::scripting::ScriptInterface;

ScriptInterface::ScriptInterface() {
    open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
}

void ScriptInterface::ParseResult(const sol::protected_function_result& result) {
    if (!result.valid()) {
        sol::error err = result;
        std::string what = err.what();
        values.push_back(fmt::format("{}", what));
        SPDLOG_LOGGER_INFO(lua_logger, "{}", what);
    }
}
void ScriptInterface::RunScript(std::string_view str) {
    ParseResult(do_string(str));
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
            values.push_back(fmt::format("{}", y));
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
        },
        [&](int y) {
            values.push_back(fmt::format("{}", y));
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
        },
        [&](double y) {
            values.push_back(fmt::format("{}", y));
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
    }));
}

int ScriptInterface::GetLength(std::string_view a) {
    return static_cast<int>((*this)[a]["len"]);
}
