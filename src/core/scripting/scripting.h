/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#pragma once

#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <sol/sol.hpp>

namespace cqsp::core::scripting {
class ScriptInterface : public sol::state {
 public:
    using sol::state::state;
    ScriptInterface();
    void RunScript(std::string_view str);
    void ParseResult(const sol::protected_function_result&);
    void RegisterDataGroup(std::string_view name);
    void Init();
    int GetLength(std::string_view);

    std::vector<std::string> values;

    std::vector<std::string> GetLogs();

 private:
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> ringbuffer_sink;
};
}  // namespace cqsp::core::scripting
