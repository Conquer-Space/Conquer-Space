#pragma once

#include <sol/sol.hpp>
#include <string_view>

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

    ~ScriptInterface(){
        printf("Destroying sol2");
    }
   private:
};
}  // namespace scripting
}  // namespace conquerspace
