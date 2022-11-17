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

#include <fstream>
#include <limits>
#include <utility>

#include <tracy/Tracy.hpp>

#include "common/components/economy.h"
#include "common/components/name.h"

void cqsp::common::systems::SysMarket::DoSystem() {
    ZoneScoped;
    // Get all the new and improved (tm) markets
    auto marketview = GetUniverse().view<components::Market>();
    SPDLOG_INFO("Processing {} market(s)", marketview.size());
    TracyPlot("Market Count", (int64_t)marketview.size());
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
        // market.ds_ratio = market.previous_demand.SafeDivision(market.supply);
        // market.ds_ratio = market.ds_ratio.Clamp(0, 2);

        for (auto& market_info : market) {
            auto& info = market_info.second;
            const double sd_ratio = info.sd_ratio();
            double& price = info.price;
            // If supply and demand = 0, then it will be undefined
            if (sd_ratio < 1) {
                // Too much demand, so we will increase the price
                // Later increase it based on SD ratio
                price += (0.02 + price * 0.01f);
                //price = 0.5;
            } else if (sd_ratio > 1 || sd_ratio == std::numeric_limits<double>::infinity()) {
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

        market.last_market_information = market.market_information;
        // Swap and clear?
        /*
        std::swap(market.supply, market.previous_supply);
        std::swap(market.demand, market.previous_demand);
        std::swap(market.latent_demand, market.last_latent_demand);

        market.supply.clear();
        market.demand.clear();
        market.latent_supply.clear();
        market.latent_demand.clear();
        */
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
            auto& mk_i = market[goodenity];
            mk_i.price = universe.get<components::Price>(goodenity);
            // Set the supply and demand things as 1 so that they sell for
            // now
            //mk_i.pre
            //market.previous_demand[goodenity] = 1;
            //market.previous_supply[goodenity] = 1;
            mk_i.supply = 1;
            mk_i.demand = 1;
            // Set previous information
            market.last_market_information[goodenity] = mk_i;
        }
        //market.history.push_back(market);
    }
}
