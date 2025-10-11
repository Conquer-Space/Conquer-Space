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

#include "common/components/bodies.h"
#include "common/components/market.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"

namespace cqsp::common::systems {
void SysInterplanetaryTrade::DoSystem() {
    auto planetary_markets = GetUniverse().view<components::Market, components::PlanetaryMarket, components::Habitation>();
    for (entt::entity entity : planetary_markets) {
        auto& market_component = GetUniverse().get<components::Market>(entity);
        // Get the S/D ratio and see if we need to make a difference
        auto& habitation = GetUniverse().get<components::Habitation>(entity);
        // Their parent market should probably have a planetary market
        auto& planetary_market = GetUniverse().get<components::PlanetaryMarket>(entity);
        planetary_market.supply_difference = market_component.demand() - market_component.supply();
    }
}

void SysInterplanetaryTrade::ParseOrbitTreeMarket(entt::entity body) {
    auto& orbital_system = GetUniverse().get<components::bodies::OrbitalSystem>(body);
    for (entt::entity child : orbital_system.children) {
        if (GetUniverse().all_of<components::PlanetaryMarket, components::bodies::OrbitalSystem>(child)) {
            ParseOrbitTreeMarket(child);
        }
    }
    // See if we can fulfill the current recipe
    // Check for space ports
    // Get space ports
}
}  // namespace cqsp::common::systems
