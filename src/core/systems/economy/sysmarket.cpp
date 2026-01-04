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
#include "core/systems/economy/sysmarket.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <utility>

#include <tracy/Tracy.hpp>

#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/spaceport.h"

namespace cqsp::core::systems {

using components::Market;

void SysMarket::DoSystem() {
    ZoneScoped;
    auto marketview = GetUniverse().nodes<Market>(entt::exclude<components::PlanetaryMarket>);

    auto goodsview = GetUniverse().nodes<components::Price>();

    for (Node market_node : marketview) {
        ZoneScoped;
        // Get the resources and process the price
        // Get demand
        Market& market = market_node.get<Market>();

        // Add a supply if there is a space port
        if (market_node.any_of<components::infrastructure::SpacePort>()) {
            // Then add output resources to the market
            auto& space_port = market_node.get<components::infrastructure::SpacePort>();
            market.supply() += space_port.output_resources_rate;

            // Remove the ones that are less than zero
            space_port.output_resources -= space_port.output_resources_rate;
            // If they're higher we set the output resouurces to zero
            for (auto& [good, value] : space_port.output_resources) {
                if (value < 0) {
                    space_port.output_resources_rate[good] = 0;
                }
            }
        }

        // TODO(EhWhoAmI): GDP Calculations
        // market.gdp = market.volume* market.price;

        // Calculate Supply and demand
        // Add combined supply and demand to compute S/D ratio
        market.supply() = market.production;
        market.demand() = market.consumption;
        market.supply().AddPositive(market.trade);
        market.demand().AddNegative(market.trade);
        market.sd_ratio = (market.supply()).SafeDivision(market.demand());

        for (Node good_node : goodsview) {
            DeterminePrice(market, good_node);
        }

        DetermineShortages(market);
    }
}

void SysMarket::DetermineShortages(components::Market& market) {
    components::ResourceMap& market_supply = market.supply();
    components::ResourceMap& market_demand = market.demand();
    double deficit = 0;
    for (auto iterator = market_supply.begin(); iterator != market_supply.end(); iterator++) {
        entt::entity good = iterator->first;
        const double& demand = market_demand[good];
        const double& supply = iterator->second;
        deficit += (demand - supply) * market.price[good];

        double shortage_level = (demand - supply) / demand;
        if (demand == 0) {
            shortage_level = 0;
        }
        // If we have too much of a shortage, we add to the shortage level
        if (shortage_level > GetUniverse().economy_config.market_config.shortage_level) {
            // The demand vs supply ratio should be below a certain amount
            market.chronic_shortages[good] += shortage_level;
        } else if (shortage_level > 0) {
            if (market.chronic_shortages.contains(good)) {
                market.chronic_shortages[good] += shortage_level;
            }
        } else {
            // We are currently recovering from a shortage
            market.chronic_shortages[good] -= std::max(market.chronic_shortages[good] - (1 - shortage_level), 0.);
        }
    }
    market.last_deficit = deficit;
    market.deficit += deficit;
}

void SysMarket::DeterminePrice(Market& market, Node& good_entity) {
    const double sd_ratio = market.sd_ratio[good_entity];
    const double supply = market.supply()[good_entity];
    const double demand = market.demand()[good_entity];
    double& price = market.price[good_entity];
    // Get parent market price
    // Now just adjust cost
    // Get parent market
    price = base_prices[good_entity] *
            (1. + GetUniverse().economy_config.market_config.base_price_deviation *
                      std::clamp((demand - supply) / (std::max(0.001, std::min(demand, supply))), -1., 1.));
}

void SysMarket::Init() {
    auto goodsview = GetUniverse().nodes<components::Price>();

    // Calculate all the things
    for (entt::entity entity : GetUniverse().nodes<Market>()) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        Market& market = GetUniverse().get<Market>(entity);

        // Initialize the price
        for (Node good_node : goodsview) {
            market.price[good_node] = good_node.get<components::Price>();
            // Set the supply and demand things as 1 so that they sell for
            // now
            market.supply()[good_node] = 1;
            market.demand()[good_node] = 1;
            market.market_access[good_node] = GetUniverse().economy_config.market_config.default_market_access;
        }
        market.sd_ratio = market.supply().SafeDivision(market.demand());
        market.history.push_back(market);
    }
    for (Node good_node : goodsview) {
        base_prices[good_node] = good_node.get<components::Price>();
    }
}
}  // namespace cqsp::core::systems
