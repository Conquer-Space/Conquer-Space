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
#include "common/systems/economy/sysplanetarytrade.h"

#include <algorithm>
#include <cmath>

#include "common/components/economy.h"
#include "common/components/surface.h"

namespace cqsp::common::systems {
void SysPlanetaryTrade::DoSystem() {
    // Sort through all the districts, and figure out their trade
    // Get all the markets
    // Then cross reference to see if they can buy or sell
    // Then list all the markets
    // Get the market of the planet, and add latent supply and demand, and then compute the market
    auto planetary_markets =
        GetUniverse().view<components::Market, components::PlanetaryMarket, components::Habitation>();

    auto goodsview = GetUniverse().view<components::Price>();

    for (entt::entity entity : planetary_markets) {
        auto& p_market = GetUniverse().get<components::Market>(entity);
        auto& habitation = GetUniverse().get<components::Habitation>(entity);
        auto& wallet = GetUniverse().get_or_emplace<components::Wallet>(entity);

        p_market.trade.clear();

        for (entt::entity habitation : habitation.settlements) {
            auto& market = GetUniverse().get<components::Market>(habitation);

            p_market.supply() += market.supply();
            p_market.demand() += market.demand();
        }

        for (entt::entity good_entity : goodsview) {
            DeterminePrice(p_market, good_entity);
        }
        // Now we can compute the prices for the individual markets
        for (entt::entity habitation : habitation.settlements) {
            auto& market = GetUniverse().get<components::Market>(habitation);
            auto& market_wallet = GetUniverse().get_or_emplace<components::Wallet>(habitation);
            for (entt::entity good_entity : goodsview) {
                market.price[good_entity] = p_market.price[good_entity] * market.market_access +
                                            (1 - market.market_access) * market.price[good_entity];
            }
        }
    }
    initial_tick = false;
}

void SysPlanetaryTrade::Init() {
    auto goodsview = GetUniverse().view<components::Price>();

    for (entt::entity good_entity : goodsview) {
        base_prices[good_entity] = GetUniverse().get<components::Price>(good_entity);
    }
}

void SysPlanetaryTrade::DeterminePrice(components::Market& market, entt::entity good_entity) {
    const double sd_ratio = market.sd_ratio[good_entity];
    const double supply = market.supply()[good_entity];
    const double demand = market.demand()[good_entity];
    double& price = market.price[good_entity];
    // Now just adjust cost
    // Get parent market
    price = base_prices[good_entity] *
            (1 + 0.75 * std::clamp((demand - supply) / (std::max(0.001, std::min(demand, supply))), -1., 1.));
}
}  // namespace cqsp::common::systems
