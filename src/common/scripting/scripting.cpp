#include "scripting.h"

#include <iostream>
#include <fmt/format.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

auto lua_logger = spdlog::stdout_color_mt("lua");

using conquerspace::scripting::ScriptInterface;

ScriptInterface::ScriptInterface() {
    open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
}

void ScriptInterface::ParseResult(sol::protected_function_result& result) {
    if (!result.valid()) {
        sol::error err = result;
        std::string what = err.what();
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
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
        },
        [&](int y) {
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
        },
        [&](double y) {
            SPDLOG_LOGGER_INFO(lua_logger, "{}", y);
    }));
}

int ScriptInterface::GetLength(std::string_view a) {
    return static_cast<int>((*this)[a]["len"]);
}
