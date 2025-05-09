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

#include "common/components/economy.h"
#include "common/components/surface.h"

void cqsp::common::systems::SysPlanetaryTrade::DoSystem() {
    // Sort through all the districts, and figure out their trade
    // Get all the markets
    // Then cross reference to see if they can buy or sell
    // Then list all the markets
    // Get the market of the planet, and add latent supply and demand, and then compute the market
    auto planetary_markets =
        GetUniverse().view<components::Market, components::PlanetaryMarket, components::Habitation>();
    for (entt::entity entity : planetary_markets) {
        auto& p_market = GetUniverse().get<components::Market>(entity);
        auto& habitation = GetUniverse().get<components::Habitation>(entity);
        auto& wallet = GetUniverse().get_or_emplace<components::Wallet>(entity);
        for (entt::entity habitation : habitation.settlements) {
            if (!GetUniverse().any_of<components::Market>(habitation)) {
                continue;
            }
            auto& market = GetUniverse().get<components::Market>(habitation);
            auto& market_wallet = GetUniverse().get_or_emplace<components::Wallet>(habitation);
            // Supply the previous market information to this
            // Add the supply and difference
            // Add supply difference to itself or something
            // How do we ensure that is is properly supplied?
            market.demand().AddPositive(market.supply_difference);
            market.supply().AddNegative(market.supply_difference);

            // Compute deficit
            double trade_balance = (market.supply_difference * p_market.price).GetSum();
            wallet += trade_balance;
            market_wallet += trade_balance;
            // Cities are the actual entities that buy and sell goods on the market
            // therefore they will have some sort of wallet to handle the trade on the global market.
            // In the future, we can have shipping companies own fractions of the shipping market
            // then we can do shipping companies or other stuff like that.
            // The issue is that what if a market runs out of money? like completely? what do we do?
            // Maybe we can implement it wity some sort of deficit or debt system, but I think that will be
            // faroff
            p_market.supply().AddPositive(market.supply_difference);
            p_market.demand().AddNegative(market.supply_difference);
            // We probably need stockpiles for more isolated markets...
            // Or do we do stockpiles for everything...
        }
        // Swap the old and new markets
        p_market.ResetLedgers();
    }
}
