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
#include <spdlog/spdlog.h>

#include "common/components/resource.h"
#include "common/components/economy.h"

namespace cqspc = cqsp::common;
namespace cqsps = cqspc::systems;
// TODO(EhWhoAmI): Optimize for all assets in market in the future;
void cqsps::market::DeterminePrice(cqsp::common::Universe& universe,
                                                    entt::entity market, entt::entity good) {
    // Get supply and demand determine if we have a lot or very little, then do some math
    double supply = universe.get<cqspc::components::ResourceStockpile>(market)[good];
    double demand = universe.get<cqspc::components::Market>(market).demand[good];
    // If it's the first time, add the first price, or else add or subtract it by S/D ratio, I guess

    // Supply-demand ratio
    // They will try to balance to a sd ratio of about 1
    auto& market_object = universe.get<cqsp::common::components::Market>(market);
    double &price = market_object.prices[good];
    double sd_ratio = supply / demand;

    // TODO(EhWhoAmI): Determine the change in pricebased on the S/D ratio so that the value will
    // be quicker to equalize

    // This can be adjusted so that the tolerance for a 'balanced' S/D ration can be wider, so that
    // prices won't fluctuate so much
    if (sd_ratio <= 1) {
        // Too much demand, so we will increase the price
        price += (0.001 + price * 0.1f);
    } else if (sd_ratio >= 1) {
        // Too much supply, so we will decrease the price
        price += (-0.001 + price * -0.1f);
        // Limit price to a minimum of 0.001
        if (price < 0.001) {
            price = 0.001;
        }
    }
}

void cqsps::market::DeterminePrices(cqsp::common::Universe& universe, entt::entity stockpile_entity) {
    auto& market = universe.get<cqspc::components::Market>(stockpile_entity);
    auto& stockpile = universe.get<cqspc::components::ResourceStockpile>(stockpile_entity);
    // Sort through demand and calculate things
    for (auto it = market.demand.begin(); it != market.demand.end(); it++) {
        // Get supply and demand determine if we have a lot or very little, then do some math
        double supply = stockpile[it->first];

        double demand = it->second;
        // If it's the first time, add the first price, or else add or subtract it by S/D ratio, I guess

        // Supply-demand ratio
        // They will try to balance to a sd ratio of about 1
        double &price = market.prices[it->first];
        double sd_ratio = -1;
        if (supply == 0 && demand == 0) {
            // Then don't change anything
            sd_ratio = 1;
        } else if (demand == 0) {
            // infinite supply, so zero demand, so set s/d ratio to infinity
            sd_ratio = std::numeric_limits<double>::infinity();
        } else if (supply == 0) {
            // Zero supply, so zero s/d ratio
            sd_ratio = 0;
        } else {
            sd_ratio = supply / demand;
        }
        assert(sd_ratio >= 0);

        market.sd_ratio[it->first] = sd_ratio;
        // TODO(EhWhoAmI): Determine the change in pricebased on the S/D ratio so that the value will
        // be quicker to equalize

        // This can be adjusted so that the tolerance for a 'balanced' S/D ration can be wider, so that
        // prices won't fluctuate so much
        if (sd_ratio <= 1) {
            // Too much demand, so we will increase the price
            price += (0.001 + price * 0.1f);
        } else if (sd_ratio >= 1) {
            // Too much supply, so we will decrease the price
            price += (-0.001 + price * -0.1f);
            // Limit price to a minimum of 0.001
            if (price < 0.001) {
                price = 0.001;
            }
        }
    }
}
