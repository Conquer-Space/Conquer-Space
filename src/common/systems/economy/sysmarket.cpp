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

#include "common/components/economy.h"

void cqsp::common::systems::SysMarket::DoSystem() {
    // Get all the new and improved (tm) markets
    auto view = GetUniverse().view<components::Market, components::ResourceStockpile>();
    // Calculate all the things
    for (entt::entity entity : view) {
        // Get the resources and process the price, then do things, I guess
        // Get demand
        auto& market = GetUniverse().get<components::Market>(entity);
        auto& supply = GetUniverse().get<components::ResourceStockpile>(entity);
        // Calculate all the goods in the map
        // Our economy will be demand driven, which means we only calculate the resources in the demand part of themap
        for (auto demand : market.current_demand) {
            double supply_count = supply[demand.first] + market.volume[demand.first];
            double sd_ratio = 1;
            if (demand.second <= 0) {
                // Negative or zero demand, then there is an infinite supply, so we don't process it?
            }
            sd_ratio = supply_count / demand.second;
            // Then we adjust the price

            // TODO(EhWhoAmI): Determine the change in price based on the S/D ratio so that the value will
            // be quicker to equalize

            // This can be adjusted so that the tolerance for a 'balanced' S/D ration can be wider, so that
            // prices won't fluctuate so much
            double& price = market.prices[demand.first];
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
        }
        // Compile supply
        market.last_information.supply.clear();
        market.last_information.supply = market.volume + supply;
        std::exchange(market.last_information.volume, market.volume);
        market.volume.clear();
        // Clear the demand
        std::exchange(market.last_information.demand, market.current_demand);
        market.current_demand.clear();
    }
}
