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
#include "common/systems/economy/sysmarket.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <utility>

#include <tracy/Tracy.hpp>

#include "common/components/economy.h"
#include "common/components/name.h"

namespace cqsp::common::systems {

using components::Market;

void SysMarket::DoSystem() {
    ZoneScoped;
    auto marketview = GetUniverse().view<Market>(entt::exclude<components::PlanetaryMarket>);
    SPDLOG_INFO("Processing {} market(s)", marketview.size_hint());
    TracyPlot("Market Count", (int64_t)marketview.size_hint());

    auto goodsview = GetUniverse().view<components::Price>();
    Universe& universe = GetUniverse();

    for (entt::entity entity : marketview) {
        // Get the resources and process the price
        // Get demand
        Market& market = universe.get<Market>(entity);

        // TODO(EhWhoAmI): GDP Calculations
        // market.gdp = market.volume* market.price;

        // Calculate Supply and demand
        // Add combined supply and demand to compute S/D ratio
        market.supply() = market.production;
        market.demand() = market.consumption;
        market.supply().AddPositive(market.trade);
        market.demand().AddNegative(market.trade);
        market.sd_ratio = (market.supply()).SafeDivision(market.demand());

        for (entt::entity good_entity : goodsview) {
            DeterminePrice(market, good_entity);
        }

        components::ResourceLedger& market_supply = market.supply();
        components::ResourceLedger& market_demand = market.demand();
        for (auto iterator = market_supply.begin(); iterator != market_supply.end(); iterator++) {
            if (iterator->second == 0 && market_demand[iterator->first] > 0) {
                market.chronic_shortages[iterator->first]++;
            } else {
                // Remove it if it exists in it
                if (market.chronic_shortages.contains(iterator->first)) {
                    market.chronic_shortages[iterator->first]--;
                    market.chronic_shortages[iterator->first] = std::max(market.chronic_shortages[iterator->first], 0.);
                }
            }
        }
    }
}

void SysMarket::DeterminePrice(Market& market, entt::entity good_entity) {
    const double sd_ratio = market.sd_ratio[good_entity];
    const double supply = market.supply()[good_entity];
    const double demand = market.demand()[good_entity];
    double& price = market.price[good_entity];
    // Now just adjust cost
    // Get parent market
    price = base_prices[good_entity] *
            (1 + 0.75 * std::clamp((demand - supply) / (std::max(0.001, std::min(demand, supply))), -1., 1.));
}

void SysMarket::Init() {
    auto marketview = GetUniverse().view<Market>();
    auto goodsview = GetUniverse().view<components::Price>();

    // Calculate all the things
    for (entt::entity entity : marketview) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        Market& market = GetUniverse().get<Market>(entity);

        // Initialize the price
        for (entt::entity good_entity : goodsview) {
            market.price[good_entity] = GetUniverse().get<components::Price>(good_entity);
            // Set the supply and demand things as 1 so that they sell for
            // now
            market.previous_demand()[good_entity] = 1;
            market.previous_supply()[good_entity] = 1;
            market.supply()[good_entity] = 1;
            market.demand()[good_entity] = 1;
        }
        market.sd_ratio = market.supply().SafeDivision(market.demand());
        market.history.push_back(market);
    }
    for (entt::entity good_entity : goodsview) {
        base_prices[good_entity] = GetUniverse().get<components::Price>(good_entity);
    }
}
}  // namespace cqsp::common::systems
