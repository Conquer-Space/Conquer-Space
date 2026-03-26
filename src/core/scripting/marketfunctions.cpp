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
#include "core/scripting/marketfunctions.h"

#include "core/components/market.h"
#include "core/scripting/functionreg.h"

// Helper function to get around sol's error
// NOLINTBEGIN: We are supposed to add parentheses around the macro parameters but it's not possible because
// the macro won't work
#define SOL_PROPERTY(type, prop_type, name) \
    sol::property(([](type& self) { return self.name; }), ([](type& self, prop_type name) { self.name = name; }))
// NOLINTEND
namespace cqsp::core::scripting {
void LoadMarketFunctions(Universe& universe, sol::state_view& script_engine) {
    CREATE_NAMESPACE(core);

    // Register orbit data
    lua_namespace.new_usertype<components::Market>("Market", sol::constructors<components::Market(size_t)>(), "GDP",
                                                   SOL_PROPERTY(components::Market, double, GDP));
    REGISTER_FUNCTION("get_market", [&](entt::entity entity) { return universe.get<components::Market>(entity); });
}
}  // namespace cqsp::core::scripting
