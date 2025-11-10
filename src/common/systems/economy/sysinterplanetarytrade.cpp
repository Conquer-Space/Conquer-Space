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
    auto planetary_markets =
        GetUniverse().nodes<components::Market, components::PlanetaryMarket, components::Habitation>();
    for (Node market_node : planetary_markets) {
        auto& market_component = market_node.get<components::Market>();
        // Get the S/D ratio and see if we need to make a difference
        auto& habitation = market_node.get<components::Habitation>();
        // Their parent market should probably have a planetary market
        auto& planetary_market = market_node.get<components::PlanetaryMarket>();
        planetary_market.supply_difference = market_component.demand() - market_component.supply();
    }

    ResolveTrades();

    // Loop through spaceports, see what we can fulfill and then compute what it is for
    for (Node space_port_node : GetUniverse().nodes<components::infrastructure::SpacePort>()) {
        auto& space_port_component = space_port_node.get<components::infrastructure::SpacePort>();
        Node planet_node = space_port_node.Convert(space_port_component.reference_body);
        auto& planetary_market = planet_node.get<components::PlanetaryMarket>();
        // Now compute the value and see if we can launch stuff
        // Let's buy the goods and then ship them
        // Loop through goods and add to the queue
        for (auto& [good, market_orders] : planetary_market.demands) {
            // Fill the space port with the queue
            // If it's not full or something
            // Add new order
            // Go through vector and pop the stack
            for (auto& order : market_orders) {
                // Add the market orders to the space port and then delete some of the goods
                space_port_component.deliveries[order.target].emplace_back(order, good);
                order.amount = 0;
            }
            market_orders.erase(std::remove_if(market_orders.begin(), market_orders.end(),
                                               [](const components::MarketOrder& order) { return order.amount <= 0; }),
                                market_orders.end());
        }
    }
}

void SysInterplanetaryTrade::ResolveTrades() {
    auto planetary_markets =
        GetUniverse().nodes<components::Market, components::PlanetaryMarket, components::Habitation>();

    for (Node seller_node : planetary_markets) {
        // Market we are going to ship from
        auto& seller_planetary_market = seller_node.get<components::PlanetaryMarket>();
        auto& seller_market = seller_node.get<components::Market>();
        for (Node buyer_node : planetary_markets) {
            if (seller_node == buyer_node) {
                continue;
            }
            // Market we are going to ship to
            auto& buyer_planetary_market = buyer_node.get<components::PlanetaryMarket>();
            auto& buyer_market = buyer_node.get<components::Market>();
            // Check for any goods, check if the price is worth it, then buy it
            // Check if the seller market needs stuff
            for (auto& [good, supply_difference] : buyer_planetary_market.supply_difference) {
                if (supply_difference <= 0) {
                    // Let's not process anything where supply is greater than demand for now
                    // TODO(EhWhoAmI): Leave a way to indicate if you want a great surplus
                    // versus a low surplus
                    continue;
                }
                // Now see if the price is acceptable
                // TODO(EhWhoAmI): Estimate the cost of the good
                if (buyer_market.price[good] > seller_market.price[good] &&
                    seller_market.chronic_shortages[good] <= 0) {
                    // We should add a market order to the buyer market and then figure out
                    // Now dump it to the market
                    components::MarketOrder order(buyer_node, supply_difference, buyer_market.price[good]);
                    seller_planetary_market.demands[good].push_back(order);
                }
            }
        }
    }
}
}  // namespace cqsp::common::systems
