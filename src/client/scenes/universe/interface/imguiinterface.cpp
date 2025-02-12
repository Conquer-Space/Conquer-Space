/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "client/scenes/universe/interface/imguiinterface.h"

namespace cqsp::client::systems {
void ImGuiInterface::Init() {
    sol::optional<std::vector<sol::table>> optional = GetScriptInterface()["interfaces"]["data"];
    if (optional.has_value()) {
        interfaces = *optional;
    }
}
void ImGuiInterface::DoUI(int delta_time) {
    for (auto& iface : interfaces) {
        sol::protected_function_result result = iface["do_ui"](iface);
        GetScriptInterface().ParseResult(result);
    }
}

void ImGuiInterface::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems