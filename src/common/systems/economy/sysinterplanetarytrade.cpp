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
#include "common/systems/economy/sysinterplanetarytrade.h"

#include "common/components/economy.h"
#include "common/components/spaceport.h"
#include "common/components/bodies.h"

namespace cqsp::common::systems {
void SysInterplanetaryTrade::DoSystem() {
    auto planetary_markets = GetUniverse().view<components::Market, components::infrastructure::SpacePort>();
    for (entt::entity entity : planetary_markets) {

        auto& market_component = GetUniverse().get<components::Market>(entity);
        auto& spaceport_component = GetUniverse().get<components::infrastructure::SpacePort>(entity);
        // Get all the goods that are lacking
        for (auto& [good, shortage] : market_component.chronic_shortages) {
            if (shortage > 0) {
                // Then we should probably add this to the spaceport queue
                // Now we also need a way to account for whatever's already in the queue so we don't queue multiple requests
                // then add it to the list...
                // Since we won't get resources fulfilled
                // See if the thing
                // Or something like that
                double expected_input = market_component.demand()[good] * shortage;
                market_component.resource_fulfilled[good] = shortage;
                // Now dump that on the interplanetary market
                ////spaceport_component.reference_body
                auto& interplanetary_market = GetUniverse().get<components::InterplanetaryMarket>(spaceport_component.reference_body);\
                // Add the resources that we want
                // Add to a spaceport in the market
                spaceport_component.demanded_resources[good] += market_component.price[good];
            }
        }
    }

    ParseOrbitTreeMarket(GetUniverse().sun);
}

void SysInterplanetaryTrade::ParseOrbitTreeMarket(entt::entity body) {
    auto& orbital_system = GetUniverse().get<components::bodies::OrbitalSystem>(body);
    for (entt::entity child : orbital_system.children) {
        if (GetUniverse().all_of<components::InterplanetaryMarket, components::bodies::OrbitalSystem>(child)) {
            ParseOrbitTreeMarket(child);
        }
    }
    // See if we can fulfill the current recipe
    // Check for space ports
    // Get space ports
}
}  // namespace cqsp::common::systems
