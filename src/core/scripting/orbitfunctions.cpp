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
#include "core/scripting/orbitfunctions.h"

#include "core/components/orbit.h"
#include "core/scripting/functionreg.h"

// Helper function to get around sol's error
// NOLINTBEGIN: We are supposed to add parentheses around the macro parameters but it's not possible because
// the macro won't work
#define SOL_PROPERTY(type, prop_type, name) \
    sol::property(([](type& self) { return self.name; }), ([](type& self, prop_type name) { self.name = name; }))
// NOLINTEND
namespace cqsp::core::scripting {

namespace bodies = components::bodies;
namespace types = components::types;

void LoadOrbitFunctions(Universe& universe, sol::state_view& script_engine) {
    CREATE_NAMESPACE(core);

    // Register orbit data
    lua_namespace.new_usertype<types::Orbit>(
        "Orbit", sol::constructors<types::Orbit()>(), "GetPeriapsis", &types::Orbit::GetPeriapsis, "eccentricity",
        SOL_PROPERTY(types::Orbit, double, eccentricity), "semi_major_axis",
        SOL_PROPERTY(types::Orbit, double, semi_major_axis), "inclination",
        SOL_PROPERTY(types::Orbit, double, inclination), "LAN", SOL_PROPERTY(types::Orbit, double, LAN), "w",
        SOL_PROPERTY(types::Orbit, double, semi_major_axis), "v", SOL_PROPERTY(types::Orbit, double, v), "GM",
        SOL_PROPERTY(types::Orbit, double, GM), "reference_body",
        SOL_PROPERTY(types::Orbit, entt::entity, reference_body));
    REGISTER_FUNCTION("get_orbit",
                      [&](entt::entity entity) -> types::Orbit& { return universe.get<types::Orbit>(entity); });
}
}  // namespace cqsp::core::scripting
