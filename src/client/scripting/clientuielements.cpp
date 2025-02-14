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
#include "client/scripting/clientuielements.h"

#include "client/scenes/universe/interface/markettable.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "common/scripting/functionreg.h"

namespace cqsp::client::scripting {
void InitClientElements(common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(client);

    REGISTER_FUNCTION("EntityTooltip", [&](entt::entity entity) { systems::gui::EntityTooltip(universe, entity); });

    REGISTER_FUNCTION("MarketInformationTable",
                      [&](entt::entity entity) { client::systems::MarketInformationTable(universe, entity); });
}
}  // namespace cqsp::client::scripting
