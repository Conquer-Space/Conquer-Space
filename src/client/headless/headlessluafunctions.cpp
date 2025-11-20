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
#include "client/headless/headlessluafunctions.h"

#include "core/scripting/functionreg.h"

namespace cqsp::client::headless {

void TickFunctions(HeadlessApplication& application) {
    core::scripting::ScriptInterface& script_engine = application.GetGame().GetScriptInterface();
    CREATE_NAMESPACE(simulation);

    REGISTER_FUNCTION("tick", [&](unsigned int ticks) {
        for (int i = 0; i < ticks; i++) {
            application.GetSimulation().tick();
        }
    });
}

void LoadHeadlessFunctions(HeadlessApplication& application) { TickFunctions(application); }
}  // namespace cqsp::client::headless
