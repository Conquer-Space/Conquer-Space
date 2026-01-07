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
#include "core/actions/economy/markethelpers.h"

#include "core/components/history.h"
#include "core/components/market.h"

namespace cqsp::core::actions {

void AddParticipant(Node& market_node, Node& participant) {
    auto& market = market_node.get<components::Market>();
    market.participants.insert(participant);
    participant.emplace<components::MarketAgent>(market_node);
    static_cast<void>(participant.get_or_emplace<components::Wallet>());
}

double GetCost(Node& market, const components::ResourceMap& ledger) {
    if (!market.any_of<components::Market>()) {
        return 0.0;
    }
    //return market.get<components::Market>().GetPrice(ledger);
}

Node CreateMarket(Universe& universe) {
    Node market(universe);
    static_cast<void>(market.get_or_emplace<components::Market>(universe.GoodCount()));
    static_cast<void>(market.get_or_emplace<components::MarketHistory>());
    return market;
}
}  // namespace cqsp::core::actions