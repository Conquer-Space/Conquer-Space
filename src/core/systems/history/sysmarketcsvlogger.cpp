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
#include "core/systems/history/sysmarketcsvlogger.h"

#include "core/components/history.h"
#include "core/components/market.h"

namespace cqsp::core::systems::history {
void SysMarketCsvHistory::Init() {}
void SysMarketCsvHistory::DoSystem() {
    // Now log every system
    auto view = GetUniverse().view<components::LogMarket, components::Market>();
    for (entt::entity entity : view) {
        // Let's copy the market
        const auto& market = GetUniverse().get<components::Market>(entity);
        //auto market_copy = market;
    }
}
}  // namespace cqsp::core::systems::history
