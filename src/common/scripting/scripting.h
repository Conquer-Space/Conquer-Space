/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <string_view>

#include <sol/sol.hpp>

namespace conquerspace {
namespace scripting {
class ScriptInterface : public sol::state {
 public:
    using sol::state::state;
    ScriptInterface();
    void RunScript(std::string_view str);
    void ParseResult(sol::protected_function_result&);
    void RegisterDataGroup(std::string_view name);
    void Init();
    int GetLength(std::string_view);
};
}  // namespace scripting
}  // namespace conquerspace
