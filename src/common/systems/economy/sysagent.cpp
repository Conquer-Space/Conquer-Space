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
#include "common/systems/economy/sysagent.h"

#include "common/actions/economy/markethelpers.h"
#include "common/components/market.h"

namespace cqsp::common::systems {
void SysAgent::DoSystem() {
    /*
    for (Node agent_node : GetUniverse().nodes<components::MarketAgent>()) {
        // Sell resources that agents produced
        double production_multiplier = 1;
        if (agent_node.any_of<components::FactoryProductivity>()) {
            auto& prod = agent_node.get<components::FactoryProductivity>();
            production_multiplier = prod.current_production;
        }
        components::ResourceLedger selling;
        
        if (agent_node.all_of<components::ResourceGenerator>()) {
            auto& gen = agent_node.get<components::ResourceGenerator>();
            selling.MultiplyAdd(gen, production_multiplier);
        }
        

        // Recipe things
        auto resource_converter = agent_node.try_get<components::ResourceConverter>();
        components::Recipe* recipe = nullptr;
        if (resource_converter != nullptr && agent_node.all_of<components::FactoryProducing>()) {
            recipe = agent_node.try_get<components::Recipe>(resource_converter->recipe);
            // Sell the recipe production
            selling.MultiplyAdd(recipe->output, production_multiplier);
            agent_node.remove<components::FactoryProducing>();
        }
        if (!selling.empty()) {
            economy::SellGood(GetUniverse(), entity, selling);
        }

        // Buy the resources that they produced
        components::ResourceLedger buying;
        if (agent_node.all_of<components::ResourceConsumption>()) {
            auto& gen = agent_node.get<components::ResourceConsumption>();
            buying.MultiplyAdd(gen, production_multiplier);
        }

        if (resource_converter != nullptr && recipe != nullptr) {
            // Sell the recipe production
            buying.MultiplyAdd(recipe->input, production_multiplier);
        }
        // Check if they buy or sell the goods, if they cannot achieve that,
        // then deal with it
        if (buying.empty()) {
            continue;
        }
        bool success = economy::PurchaseGood(GetUniverse(), entity, buying);
        if (success) {
            GetUniverse().emplace_or_replace<components::FactoryProducing>(entity);
        }
    }
    */
}
}  // namespace cqsp::common::systems