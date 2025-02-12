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
#include "client/scripting/imguifunctions.h"

#include <imgui.h>

#include "common/scripting/functionreg.h"
namespace cqsp::client::scripting {
void LoadImGuiFunctions(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(ImGui);

    REGISTER_FUNCTION("Begin", [](const char* name, bool p_open, ImGuiWindowFlags flags = 0) {
        return ImGui::Begin(name, &p_open, flags);
    });

    REGISTER_FUNCTION("End", []() { return ImGui::End(); });

    REGISTER_FUNCTION("Text", [](const char* fmt) {
        ImGui::Text(fmt);  // Since we don't have reflection, we can't do
                           // varargs.
    });

    REGISTER_FUNCTION("Separator", []() { ImGui::Separator(); });
}
}  // namespace cqsp::client::scripting
