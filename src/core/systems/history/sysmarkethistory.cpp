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
#include "core/systems/history/sysmarkethistory.h"

#include "core/components/history.h"

namespace cqsp::core::systems::history {
void SysMarketHistory::DoSystem() {
    auto planetary_markets =
        GetUniverse().view<components::Market, components::PlanetaryMarket, components::MarketHistory>();

    for (auto&& [entity, market, planetary_market, history] : planetary_markets.each()) {
        // Now let's get or emplace our market history?
        // then add to the market history
        history.gdp.push_back(market.GDP);
        for (auto good : GetUniverse().GoodIterator()) {
            history.price_history[static_cast<int>(good)].push_back(market.price[good]);
            history.sd_ratio[static_cast<int>(good)].push_back(market.sd_ratio[good]);
            history.supply[static_cast<int>(good)].push_back(market.supply[good]);
            history.demand[static_cast<int>(good)].push_back(market.demand[good]);
        }
    }
}
void SysMarketHistory::Init() {}
}  // namespace cqsp::core::systems::history
