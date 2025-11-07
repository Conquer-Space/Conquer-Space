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
#include "common/systems/history/sysmarkethistory.h"

#include "common/components/history.h"
#include "common/components/market.h"
namespace cqsp::common::systems::history {

using components::Market;
using components::MarketHistory;

void SysMarketHistory::DoSystem() {
    /*
    for (Node market_node : GetUniverse().nodes<Market>()) {
        Market& market_data = market_node.get<Market>();
        market_data.history.push_back(market_data);
    }
    auto view = GetUniverse().nodes<Market, MarketHistory>();
    for (Node market_node : view) {
        auto& history = market_node.get<MarketHistory>();
        Market& market_data = market_node.get<Market>();
        // Loop through the prices
        for (auto resource : market_data.market_information) {
            history.price_history[resource.first].push_back(resource.second.price);
            history.volume[resource.first].push_back(market_data.last_market_information[resource.first].demand);
        }
        double val = 0;
        for (Node participant_node : market_node.Convert(market_data.participants)) {
            if (participant_node.any_of<components::Wallet>()) {
                auto& wallet = participant_node.get<components::Wallet>();
                val += wallet.GetGDPChange();
            }
        }
        history.gdp.push_back(val);
    */
}
}  // namespace cqsp::common::systems::history
