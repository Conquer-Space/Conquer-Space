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

#include "common/components/economy.h"

void cqsp::common::systems::SysMarket::DoSystem() {
    // Get all the new and improved (tm) markets
    auto marketview = GetUniverse().view<components::Market>();
    auto goodsview = GetUniverse().view<components::Price>();
    Universe& universe = GetUniverse();
    // Calculate all the things
    for (entt::entity entity : marketview) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        components::Market& market = universe.get<components::Market>(entity);
        components::ResourceStockpile& stockpile =
            universe.get_or_emplace<components::ResourceStockpile>(entity);
        // Calculate all the goods in the map
        // Our economy will be demand driven, which means we only calculate the resources in the demand part of themap
        // Get the resources that are traded, I guess
        market.sd_ratio = market.supply.SafeDivision(market.demand);
        market.ds_ratio = market.demand.SafeDivision(market.supply);
        //market.ds_ratio = market.ds_ratio.Clamp(0, 2);
        if (market.history.size() == 0) 
        {
            for (entt::entity goodenity : goodsview) {
                market.price[goodenity] =
                    universe.get<components::Price>(goodenity);
            }
        }

        for (entt::entity goodenity : goodsview) 
        {
            const double sd_ratio = market.sd_ratio[goodenity];
            double price = market.price[goodenity];

            if (sd_ratio < 1) {
                // Too much demand, so we will increase the price
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
                //price = 2;
            } else {
                // Keep price approximately the same
            }
            market.price[goodenity] = price;
        }
        components::MarketInformation current = market;
        market.history.push_back(current);

        stockpile += market.supply;
        market.supply.clear();
        stockpile -= market.demand;
        market.demand.clear();

        // Compile supply
        //std::swap(market.last_market_information, market.market_information);
    }
}
