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
#include "common/systems/economy/markethelpers.h"

#include "common/components/economy.h"
void cqsp::common::systems::economy::AddParticipant(
                cqsp::common::Universe& universe, entt::entity market_entity,
                entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    auto& market = universe.get<cqspc::Market>(market_entity);
    market.participants.insert(entity);
    universe.emplace<cqspc::MarketAgent>(entity, market_entity);
}


entt::entity cqsp::common::systems::economy::CreateMarket(Universe& universe) {
    entt::entity market = universe.create();
    CreateMarket(universe, market);
    return market;
}

void cqsp::common::systems::economy::CreateMarket(Universe& universe, entt::entity market) {
    universe.get_or_emplace<components::ResourceStockpile>(market);
    universe.get_or_emplace<components::Market>(market);
}

double cqsp::common::systems::economy::PurchaseGood(Universe& universe, entt::entity agent,
    components::ResourceLedger purchase) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    entt::entity market = universe.get<components::MarketAgent>(agent).market;
    auto& market2 = universe.get<components::Market>(market);
    double cost = purchase.MultiplyAndGetSum(market2.prices);
    market2.current_demand += purchase;

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    auto& wallet = universe.get<components::Wallet>(agent);

    auto& market_stockpile = universe.get<components::ResourceStockpile>(market);
    // TODO(EhWhoAmI):
    // Check if there are enough resources on the market, or else there will have a shortage
    // Then get the maximum resources that they allow
    // Just allow agents to take as many resources as they want because I'm too lazy to implement
    // limitations.
    if (market_stockpile.EnoughToTransfer(purchase) || true) {
        // Then get the cost
        if (wallet.GetBalance() >= cost) {
            // Then agent has enough money to buy
            // Remove resources from market
            market_stockpile -= purchase;
            universe.get<components::ResourceStockpile>(agent) += purchase;
            // Add value to volume, because we need the volume
            market2.volume += purchase;
            wallet -= cost;
        }
    } else {
        // Get the resources remaining in the market
        // Calculate the resources needed
        // Get the volume and other things, somehow
        components::ResourceLedger amount_purchased = market_stockpile.LimitedRemoveResources(purchase);
        universe.get<components::ResourceStockpile>(agent) += amount_purchased;
        market2.volume += amount_purchased;
    }
    return cost;
}

double cqsp::common::systems::economy::SellGood(Universe& universe, entt::entity agent,
                                       components::ResourceLedger selling) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    entt::entity market = universe.get<components::MarketAgent>(agent).market;
    auto& market_comp = universe.get<components::Market>(market);
    auto& supply = universe.get<components::ResourceStockpile>(market);
    auto& agent_stockpile = universe.get<components::ResourceStockpile>(agent);
    supply += selling;
    // Remove from stockpile
    agent_stockpile -= selling;

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    double cost = selling.MultiplyAndGetSum(market_comp.prices);
    auto& wallet = universe.get<components::Wallet>(agent);
    wallet += cost;
    return cost;
}
