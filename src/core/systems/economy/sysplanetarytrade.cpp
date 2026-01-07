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
#include "core/systems/economy/sysplanetarytrade.h"

#include <algorithm>
#include <cmath>

#include <tracy/Tracy.hpp>

#include "core/components/market.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
void SysPlanetaryTrade::DoSystem() {
    ZoneScoped;
    // Sort through all the districts, and figure out their trade
    // Get all the markets
    // Then cross reference to see if they can buy or sell
    // Then list all the markets
    // Get the market of the planet, and add latent supply and demand, and then compute the market
    auto planetary_markets =
        GetUniverse().nodes<components::Market, components::PlanetaryMarket, components::Settlements>();

    auto goodsview = GetUniverse().nodes<components::Price>();

    for (Node market_node : planetary_markets) {
        ZoneScoped;
        auto& p_market = market_node.get<components::Market>();
        auto& habitation = market_node.get<components::Settlements>();
        auto& wallet = market_node.get_or_emplace<components::Wallet>();

        p_market.trade.clear();

        for (Node settlement_node : market_node.Convert(habitation.provinces)) {
            auto& market = settlement_node.get<components::Market>();

            p_market.supply() += market.production;
            p_market.demand() += market.consumption;

            if (settlement_node.any_of<components::infrastructure::SpacePort>()) {
                auto& space_port = settlement_node.get<components::infrastructure::SpacePort>();
                p_market.supply() += space_port.output_resources_rate;
                p_market.demand() += space_port.demanded_resources_rate;
            }
        }

        for (auto good_node : GetUniverse().GoodIterator()) {
            DeterminePrice(p_market, good_node);
        }
        // Now we can compute the prices for the individual markets
        for (Node settlement_node : market_node.Convert(habitation.provinces)) {
            auto& market = settlement_node.get<components::Market>();
            auto& market_wallet = settlement_node.get_or_emplace<components::Wallet>();
            for (auto good_node : GetUniverse().GoodIterator()) {
                double access = market.market_access[good_node];
                market.price[good_node] = p_market.price[good_node] * access + (1 - access) * market.price[good_node];
            }

            // Determine supply and demand for the market
            market.trade.clear();
            for (auto good : GetUniverse().GoodIterator()) {
                if (p_market.supply()[good] == 0) {
                    continue;
                }
                // Remove local production so that we don't confound this with our local production
                market.trade[good] -=
                    std::max((market.supply()[good] / p_market.supply()[good] * p_market.demand()[good]) -
                                 market.consumption[good],
                             0.);
            }

            for (auto good : GetUniverse().GoodIterator()) {
                if (p_market.demand()[good] == 0) {
                    continue;
                }
                // Remove local consumption so that we don't confound this with local production
                market.trade[good] +=
                    std::max((market.demand()[good] / p_market.demand()[good] * p_market.supply()[good]) -
                                 market.production[good],
                             0.);
            }
        }

        auto& planetary_market = market_node.get<components::PlanetaryMarket>();
        planetary_market.supplied_resources.clear();
    }
    initial_tick = false;
}

void SysPlanetaryTrade::Init() {
    for (auto good : GetUniverse().GoodIterator()) {
        base_prices[good] = GetUniverse().get<components::Price>(good);
    }
}

void SysPlanetaryTrade::DeterminePrice(components::Market& market, components::GoodEntity good_entity) {
    const double sd_ratio = market.sd_ratio[good_entity];
    const double supply = market.supply()[good_entity];
    const double demand = market.demand()[good_entity];
    double& price = market.price[good_entity];
    // Now just adjust cost
    // Get parent market
    price = base_prices[good_entity] *
            (1 + 0.75 * std::clamp((demand - supply) / (std::max(0.001, std::min(demand, supply))), -1., 1.));
}
}  // namespace cqsp::core::systems
