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
    return market.get<components::Market>().GetPrice(ledger);
}

Node CreateMarket(Universe& universe) {
    Node market(universe);
    static_cast<void>(market.get_or_emplace<components::Market>());
    static_cast<void>(market.get_or_emplace<components::MarketHistory>());
    return market;
}

bool PurchaseGood(Node& agent, const components::ResourceMap& purchase) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    Node market(agent.universe(), agent.get<components::MarketAgent>().market);
    auto& market_comp = market.get<components::Market>();
    // Prices
    double cost = market_comp.GetPrice(purchase);

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    auto& wallet = agent.get<components::Wallet>();

    // TODO(EhWhoAmI):
    // Check if there are enough resources on the market, or else there will have a shortage
    // Then get the maximum resources that they allow
    // Just allow agents to take as many resources as they want because I'm too lazy to implement
    // limitations.
    if (wallet.GetBalance() < cost) {
        return false;
    }

    // Then agent has enough money to buy
    market_comp.AddDemand(purchase);
    if (agent.all_of<components::ResourceStockpile>()) {
        agent.get<components::ResourceStockpile>() += purchase;
    }
    wallet -= cost;
    return true;
}

bool SellGood(Node& agent, const components::ResourceMap& selling) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    Node market(agent.universe(), agent.get<components::MarketAgent>().market);
    auto& market_comp = market.get<components::Market>();
    auto& agent_stockpile = agent.get<components::ResourceStockpile>();
    market_comp.AddSupply(selling);

    double cost = market_comp.GetPrice(selling);

    // Remove from stockpile
    agent_stockpile -= selling;

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    components::Wallet& wallet = agent.get<components::Wallet>();
    wallet += cost;
    return true;
}

}  // namespace cqsp::core::actions