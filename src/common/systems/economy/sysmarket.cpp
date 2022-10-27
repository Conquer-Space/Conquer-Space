/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <limits>
#include <utility>
#include <fstream>

#include <tracy/Tracy.hpp>

#include "common/components/economy.h"
#include "common/components/name.h"

void cqsp::common::systems::SysMarket::DoSystem() {
    ZoneScoped;
    // Get all the new and improved (tm) markets
    auto marketview = GetUniverse().view<components::Market>();
    SPDLOG_INFO("Processing {} market(s)", marketview.size());
    TracyPlot("Market Count", (int64_t) marketview.size());
    auto goodsview = GetUniverse().view<components::Price>();
    Universe& universe = GetUniverse();
    // Calculate all the things
    for (entt::entity entity : marketview) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        components::Market& market = universe.get<components::Market>(entity);

        // TODO(EhWhoAmI): GDP Calculations
        // market.gdp = market.volume* market.price;

        // Calculate Supply and demand
        market.sd_ratio = market.supply.SafeDivision(market.demand);
        // market.ds_ratio = market.previous_demand.SafeDivision(market.supply);
        // market.ds_ratio = market.ds_ratio.Clamp(0, 2);

        for (entt::entity good_entity : goodsview) {
            const double sd_ratio = market.sd_ratio[good_entity];
            double& price = market.price[good_entity];
            // If supply and demand = 0, then it will be undefined
            if (sd_ratio < 1) {
                // Too much demand, so we will increase the price
                // Later increase it based on SD ratio
                price += (0.02 + price * 0.01f);
                //price = 0.5;
            } else if (sd_ratio > 1 ||
                       sd_ratio == std::numeric_limits<double>::infinity()) {
                // Too much supply, so we will decrease the price
                price += (-0.01 + price * -0.01f);

                // Limit price to a minimum of 0.001
                if (price < 0.00001) {
                    price = 0.00001;
                }
            } else {
                // Keep price approximately the same
            }
        }
        // Set the previous supply and demand
        //components::MarketInformation current = market;
        //market.history.push_back(current);

        // Swap and clear?
        std::swap(market.supply, market.previous_supply);
        std::swap(market.demand, market.previous_demand);
        std::swap(market.latent_demand, market.last_latent_demand);

        market.supply.clear();
        market.demand.clear();
        market.latent_supply.clear();
        market.latent_demand.clear();

        // Every tick, dump the data
        auto goodsview = GetUniverse().view<components::Price>();

        // Output to file, I guess
        std::ofstream data;
        data.open(fmt::format("{}.txt", universe.GetDate()));
        data << "Good, Price, Supply, Demand, S/D Ratio, Latent Demand"
             << std::endl;
        for (entt::entity good_entity : goodsview) {
            data << GetUniverse()
                        .get<components::Identifier>(good_entity)
                        .identifier
                 << ", " << market.price[good_entity]
                 << ", " << market.previous_supply[good_entity] << ", "
                 << market.previous_demand[good_entity] << ","
                 << market.sd_ratio[good_entity] << ", "
                 << market.last_latent_demand[good_entity] << std::endl;
        }
    }
}

void cqsp::common::systems::SysMarket::InitializeMarket(Game& game) {
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
            market.price[goodenity] =
                universe.get<components::Price>(goodenity);
            // Set the supply and demand things as 1 so that they sell for
            // now
            market.previous_demand[goodenity] = 1;
            market.previous_supply[goodenity] = 1;
            market.supply[goodenity] = 1;
            market.demand[goodenity] = 1;
        }
        market.sd_ratio = market.supply.SafeDivision(market.demand);
        market.history.push_back(market);
    }
}
