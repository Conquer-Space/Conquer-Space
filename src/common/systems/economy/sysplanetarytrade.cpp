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
        p_market.trade.clear();
        for (entt::entity habitation : habitation.settlements) {
            if (!GetUniverse().any_of<components::Market>(habitation)) {
                continue;
            }
            auto& market = GetUniverse().get<components::Market>(habitation);
            auto& market_wallet = GetUniverse().get_or_emplace<components::Wallet>(habitation);
            // Import the missing goods that we need
            // We should ramp up and down imports and exports, and also try to maintain S/D ratios at 1.
            // If it's the initial tick then we just set the values, otherwise we change it by a delta
            if (initial_tick) {
                market.trade = market.supply_difference;
            } else {
                // Now we want to increase and reduce that export and import amount based off what
                // We're producing and what's on the market
                // We also want to alter the price.
                // We can set the price of the market to track the current market price
                // So we need to compute this
                // So the goal of the market is to balance the two
                // The issue with reducing the supply is that it won't lower the cost in
                // What if we did a price thing
                // But the thing is
                // The price delta also has to be proportional to the amount bought and sold
                // Now compute the proportion the price will be affected
                // If there isn't enough demand, we need to reduce the amount that we import
                // Let's just get the weighted average of the values?
                // Now compute the amount needed to
                // Let's compute the imports vs the exports of the market
                // For each element check if it's higher and then tweak the market exports and imports
                // Go through the supply difference
                // New
                for (auto it = market.supply_difference.begin(); it != market.supply_difference.end(); it++) {
                    // Now compute the difference in the parent market and reduce supply and demand based off whatever
                    // metrics
                    entt::entity good = it->first;
                    double planetary_cost = p_market.price[good];
                    double local_cost = market.price[good];
                    if (planetary_cost < local_cost) {
                        // We should import more and export less
                        // We should change the value
                        // Let's just decrease proportionally (we can do a different kind of loop)
                        // later
                        // We need to modify by trade
                        market.trade[good] *= 1. + std::clamp(market.trade[good] * 0.0001, 0., 0.1);
                    } else if (planetary_cost > local_cost) {
                        // We should export more and import less
                        market.trade[good] *= 1 - std::clamp(std::fabs(market.trade[good] * 0.0001), 0., 0.1);
                    } else {
                        // Now we should keep the same
                    }
                }
                // market.production/market.exports;

                //(market.price - p_market.price);
            }
            // Add the market inputs
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
            // The thing is that we might have an issue
            p_market.trade += market.trade;
            // We probably need stockpiles for more isolated markets...
            // Or do we do stockpiles for everything...
        }
        // Swap the old and new markets
        p_market.ResetLedgers();
    }
    initial_tick = false;
}
