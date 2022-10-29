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
#include "common/systems/history/sysmarkethistory.h"

#include "common/components/economy.h"
#include "common/components/history.h"

void cqsp::common::systems::history::SysMarketHistory::DoSystem() {
    for (entt::entity marketentity : GetUniverse().view<components::Market>()) {
        components::Market& market_data =
            GetUniverse().get<components::Market>(marketentity);
        market_data.history.push_back(market_data);
    }
    auto view = GetUniverse().view<components::Market, components::MarketHistory>();
    for (entt::entity entity : view) {
        auto& history = GetUniverse().get<components::MarketHistory>(entity);
        components::Market& market_data =
            GetUniverse().get<components::Market>(entity);
        // Loop through the prices
        for (auto resource : market_data.market_information) {
            history.price_history[resource.first].push_back(resource.second.price);
            history.volume[resource.first].push_back(
                market_data.last_market_information[resource.first].demand);
        }
        double val = 0;
        for (entt::entity ent : market_data.participants) {
            if (GetUniverse().any_of<components::Wallet>(ent)) {
                auto& wallet = GetUniverse().get<components::Wallet>(ent);
                val += wallet.GetGDPChange();
            }
        }
        history.gdp.push_back(val);
    }
}
