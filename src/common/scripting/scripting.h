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
    void RegisterDataGroup(std::string_view name);
    void Init();
    int GetLength(std::string_view);

   private:
};
}  // namespace scripting
}  // namespace conquerspace
