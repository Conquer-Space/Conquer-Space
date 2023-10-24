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
#include "common/systems/economy/systrade.h"

#include "common/components/economy.h"

void cqsp::common::systems::SysTrade::DoSystem() {
    // Sort through all the districts, and figure out their trade
    // Get all the markets
    // Then cross reference to see if they can buy or sell
    // Then list all the markets
    auto markets = GetUniverse().view<components::Market>();
    for (entt::entity entity : markets) {
        auto& market = GetUniverse().get<components::Market>(entity);
        // Iterate through the connected markets, and also calculate if we need to connect to other markets
        for (entt::entity connected : market.connected_markets) {
            // Get the entity?
        }
        // Get the prices of everything and then
        // Look for trading options
        for (entt::entity pair : markets) {
            // Get the lacking goods, and then compensentate
            if (pair == entity) {
                continue;
            }
            auto& connected_market = GetUniverse().get<components::Market>(pair);
            // Get the goods and sort through?
            // Get surplus
            //market.latent_supply;
            //market.latent_demand;
            // Get the latent supply and demand from the opposing market, and see if our latent can fulfull the other
            connected_market.latent_demand - market.latent_supply;  // Remaining goods?
        }
    }
}
