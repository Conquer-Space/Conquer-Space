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

#include <fstream>
#include <limits>
#include <utility>

#include <tracy/Tracy.hpp>

#include "common/components/economy.h"
#include "common/components/name.h"

namespace cqsp::common::systems {
void SysMarket::DoSystem() {
    ZoneScoped;
    // Get all the new and improved (tm) markets
    auto marketview = GetUniverse().view<components::Market>();
    SPDLOG_INFO("Processing {} market(s)", marketview.size());
    TracyPlot("Market Count", (int64_t)marketview.size());
    auto goodsview = GetUniverse().view<components::Price>();
    Universe& universe = GetUniverse();

    for (entt::entity entity : marketview) {
        // Get the resources and process the price
        // Get demand
        components::Market& market = universe.get<components::Market>(entity);

        // TODO(EhWhoAmI): GDP Calculations
        // market.gdp = market.volume* market.price;

        // Calculate Supply and demand
        market.sd_ratio = market.supply().SafeDivision(market.demand());
        // market.ds_ratio = market.previous_demand.SafeDivision(market.supply);
        // market.ds_ratio = market.ds_ratio.Clamp(0, 2);

        for (entt::entity good_entity : goodsview) {
            DeterminePrice(market, good_entity);
        }
        // Set the previous supply and demand
        //components::MarketInformation current = market;
        //market.history.push_back(current);
        // So positive values are left over goods in the market
        // and negative values are goods that are in demand in the market
        market.supply_difference = market.supply() - market.demand();
        // Swap and clear?
        market.ResetLedgers();
        std::swap(market.latent_demand, market.last_latent_demand);

        market.latent_supply.clear();
        market.latent_demand.clear();
    }
}

void SysMarket::DeterminePrice(components::Market& market, entt::entity good_entity) {
    const double sd_ratio = market.sd_ratio[good_entity];
    double& price = market.price[good_entity];
    // If supply and demand = 0, then it will be undefined
    if (sd_ratio < 1) {
        // Too much demand, so we will increase the price
        // Later increase it based on SD ratio
        price += (0.02 + price * 0.01f);
        //price = 0.5;
    } else if (sd_ratio > 1 || sd_ratio == std::numeric_limits<double>::infinity()) {
        // Too much supply, so we will decrease the price
        price += (-0.01 + price * -0.01f);

        // Limit price to a minimum of 0.00001
        if (price < 0.00001) {
            price = 0.00001;
        }
    } else {
        // Keep price approximately the same
    }
}

void SysMarket::InitializeMarket(Game& game) {
    auto marketview = game.GetUniverse().view<components::Market>();
    auto goodsview = game.GetUniverse().view<components::Price>();

    Universe& universe = game.GetUniverse();
    // Calculate all the things
    for (entt::entity entity : marketview) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        components::Market& market = universe.get<components::Market>(entity);

        // Initialize the price
        for (entt::entity goodenity : goodsview) {
            market.price[goodenity] = universe.get<components::Price>(goodenity);
            // Set the supply and demand things as 1 so that they sell for
            // now
            market.previous_demand()[goodenity] = 1;
            market.previous_supply()[goodenity] = 1;
            market.supply()[goodenity] = 1;
            market.demand()[goodenity] = 1;
        }
        market.sd_ratio = market.supply().SafeDivision(market.demand());
        market.history.push_back(market);
    }
}
}  // namespace cqsp::common::systems
