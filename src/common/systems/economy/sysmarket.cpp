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
    auto view = GetUniverse().view<components::Market>();
    // Calculate all the things
    for (entt::entity entity : view) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        auto& market = GetUniverse().get<components::Market>(entity);
        // Calculate all the goods in the map
        // Our economy will be demand driven, which means we only calculate the resources in the demand part of themap
        // Get the resources that are traded, I guess
        for (auto& info : market) {
            auto& market_element = info.second;
            if (market_element.supply == 0 && market_element.demand == 0) {
                continue;
            }
            double sd_ratio = market_element.supply / market_element.demand;

            if (market_element.demand <= 0) {
                // Then there is infinite supply, and the price will go down.
                sd_ratio = std::numeric_limits<double>::infinity();
            }

            double& price = market_element.price;
            if (sd_ratio < 1) {
                // Too much demand, so we will increase the price
                price += (0.001 + price * 0.1f);
            } else if (sd_ratio > 1) {
                // Too much supply, so we will decrease the price
                price += (-0.001 + price * -0.1f);
                // Limit price to a minimum of 0.001
                if (price < 0.001) {
                    price = 0.001;
                }
            } else {
                // Keep price approximately the same
            }
            // Set last market information for documentation purposes
            auto& el = market.last_market_information[info.first];
            el = market_element;
            market_element.supply = 0;
            market_element.demand = 0;
            market_element.sd_ratio = sd_ratio;
        }
        // Compile supply
        //std::swap(market.last_market_information, market.market_information);
    }
}
