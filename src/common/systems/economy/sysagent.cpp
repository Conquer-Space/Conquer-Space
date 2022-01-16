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
#include "common/systems/economy/sysagent.h"

#include "common/components/economy.h"
#include "common/systems/economy/markethelpers.h"

void cqsp::common::systems::SysAgent::DoSystem() {
    auto view = GetUniverse().view<cqsp::common::components::MarketAgent>();
    for (entt::entity entity : view) {
        // Sell resources that agents produced
        double production_multiplier = 1;
        if (GetUniverse().any_of<components::FactoryProductivity>(entity)) {
            auto& prod = GetUniverse().get<components::FactoryProductivity>(entity);
            production_multiplier = prod.current_production;
        }
        components::ResourceLedger selling;
        if (GetUniverse().all_of<components::ResourceGenerator>(entity)) {
            auto& gen = GetUniverse().get<components::ResourceGenerator>(entity);
            selling.MultiplyAdd(gen, production_multiplier);
        }
        // Recipe things
        auto resource_converter = GetUniverse().try_get<components::ResourceConverter>(entity);
        components::Recipe* recipe = nullptr;
        if (resource_converter != nullptr) {
            recipe = GetUniverse().try_get<components::Recipe>(resource_converter->recipe);
            // Sell the recipe production
            selling.MultiplyAdd(recipe->output, production_multiplier);
        }
        economy::SellGood(GetUniverse(), entity, selling);

        // Buy the resources that they produced
        components::ResourceLedger buying;
        if (GetUniverse().all_of<components::ResourceConsumption>(entity)) {
            auto& gen = GetUniverse().get<components::ResourceConsumption>(entity);
            buying.MultiplyAdd(gen, production_multiplier);
        }

        if (resource_converter != nullptr) {
            // Sell the recipe production
            buying.MultiplyAdd(recipe->input, production_multiplier);
        }
        // Check if they buy or sell the goods, if they cannot achieve that,
        // then deal with it.
        bool success = economy::PurchaseGood(GetUniverse(), entity, buying);
        if (!success) {
            SPDLOG_INFO("Cannot buy stuff");
        }
    }
}
