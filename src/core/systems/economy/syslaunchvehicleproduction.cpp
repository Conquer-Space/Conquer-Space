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
#include "core/systems/economy/syslaunchvehicleproduction.h"

#include "core/components/spaceport.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
void SysLaunchVehicleProduction::DoSystem() {
    // List cities with launch pads and then add the cost to the economy above...
    auto view = GetUniverse().view<components::City, components::infrastructure::SpacePort>();
    for (entt::entity entity : view) {
        const auto& city_comp = GetUniverse().get<components::City>(entity);
        auto& space_port_comp = GetUniverse().get<components::infrastructure::SpacePort>(entity);
        // Now we get the parent universe and then compute that as well
        entt::entity province = city_comp.province;
        if (!GetUniverse().valid(province)) {
            continue;
        }
        // Otherwise we process the province as normal
        // Compile the list of projects that we need and then add that demand to the universe
        // and also output the value into whatever queue that we have
    }
}

int SysLaunchVehicleProduction::Interval() { return ECONOMIC_TICK; }
}  // namespace cqsp::core::systems
