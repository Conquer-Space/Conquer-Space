/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "common/components/history.h"

namespace components = cqsp::common::components;
using components::Market;
using components::MarketAgent;
using components::MarketHistory;
using components::Wallet;
using components::ResourceLedger;
using components::ResourceStockpile;

namespace cqsp::common::systems::economy
{
void AddParticipant(Universe& universe, entt::entity market_entity,
                                                    entt::entity entity) {
    auto& market = universe.get<Market>(market_entity);
    market.participants.insert(entity);
    universe.emplace<MarketAgent>(entity, market_entity);
    static_cast<void>(universe.get_or_emplace<Wallet>(entity));
}

double GetCost(Universe& universe, entt::entity market,
                                               const components::ResourceLedger& ledger) {
    if (!universe.any_of<Market>(market)) {
        return 0.0;
    }
    return universe.get<Market>(market).GetPrice(ledger);
}

entt::entity CreateMarket(Universe& universe) {
    entt::entity market = universe.create();
    CreateMarket(universe, market);
    return market;
}

void CreateMarket(Universe& universe, entt::entity market) {
    static_cast<void>(universe.get_or_emplace<Market>(market));
    static_cast<void>(universe.get_or_emplace<MarketHistory>(market));
}

bool PurchaseGood(Universe& universe, entt::entity agent,
                                                  const ResourceLedger& purchase) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    entt::entity market = universe.get<MarketAgent>(agent).market;
    auto& market_comp = universe.get<Market>(market);
    // Prices
    double cost = market_comp.GetPrice(purchase);

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    auto& wallet = universe.get<Wallet>(agent);

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
    if (universe.all_of<ResourceStockpile>(agent)) {
        universe.get<ResourceStockpile>(agent) += purchase;
    }
    wallet -= cost;
    return true;
}

bool SellGood(Universe& universe, entt::entity agent,
                                              const components::ResourceLedger& selling) {
    // Calculating on how to buy from the market shouldn't be too hard, right?
    // Get the market connected to, and build the demand
    entt::entity market = universe.get<MarketAgent>(agent).market;
    auto& market_comp = universe.get<Market>(market);
    auto& agent_stockpile = universe.get<ResourceStockpile>(agent);
    market_comp.AddSupply(selling);

    double cost = market_comp.GetPrice(selling);

    // Remove from stockpile
    agent_stockpile -= selling;

    // Then subtract the cash from the person, or something
    // Check if they have enough money and purchase, I guess
    Wallet& wallet = universe.get<Wallet>(agent);
    wallet += cost;
    return true;
}
}
    

