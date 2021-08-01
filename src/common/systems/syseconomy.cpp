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
#include "common/systems/syseconomy.h"

#include "common/components/resource.h"
#include "common/components/economy.h"
#include "common/components/area.h"

#include "common/util/profiler.h"

using conquerspace::common::systems::SysFactory;
void SysFactory::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    // Produce resources
    BEGIN_TIMED_BLOCK(Resource_gen);
    SysResourceGenerator(universe);
    SysProduction(universe);
    SysConsumption(universe);
    END_TIMED_BLOCK(Resource_gen);
    BEGIN_TIMED_BLOCK(Market_sim);
    SysFactoryDemandCreator(universe);
    SysGoodSeller(universe);
    SysDemandResolver(universe);
    END_TIMED_BLOCK(Market_sim);
    BEGIN_TIMED_BLOCK(Production_sim);
    SysProductionStarter(universe);
    END_TIMED_BLOCK(Production_sim);
}

void SysFactory::SysResourceGenerator(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    auto view = universe.view<cqspc::ResourceGenerator, cqspc::ResourceStockpile>();
    for (auto [entity, production, stockpile] : view.each()) {
        // Make the resources, and dump to market
        // resources generated:
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile += production* productivity* Interval();
    }
}

void SysFactory::SysProduction(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    auto view = universe.view<cqspc::Production, cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (auto [entity, production, converter, stockpile] : view.each()) {
        // Make the resources, and dump to market
        auto& recipe = universe.get<cqspc::Recipe>(converter.recipe);
        // resources generated:
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile += recipe.output* productivity* Interval();
        // Produced, so remove the production
        universe.remove<cqspc::Production>(entity);
    }
}

void SysFactory::SysConsumption(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    auto consumption_view = universe.view<cqspc::ResourceConsumption, cqspc::MarketParticipant>();
    // Demand for next time
    for (auto [entity, consumption, participant] : consumption_view.each()) {
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto &demand = universe.emplace<cqspc::ResourceDemand>(entity);
        demand += consumption* productivity* Interval();
    }
}

void SysFactory::SysFactoryDemandCreator(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    auto demand_view = universe.view<cqspc::ResourceConverter, cqspc::MarketParticipant>();
    // Demand for next time
    for (auto [entity, converter, participant] : demand_view.each()) {
        auto& recipe = universe.get<cqspc::Recipe>(converter.recipe);

        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto &demand = universe.emplace<cqspc::ResourceDemand>(entity);
        demand += recipe.input* productivity* Interval();
    }
}

void SysFactory::SysGoodSeller(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;

    // Sell all goods
    auto good_seller = universe.view<cqspc::ResourceStockpile, cqspc::MarketParticipant>();
    // Demand for next time
    for (auto [entity, stockpile, market_participant] : good_seller.each()) {
        auto& market = universe.get<cqspc::Market>(market_participant.market);
        // Add to supply
        market.supply += stockpile;
        auto& market_stockpile = universe.get<cqspc::ResourceStockpile>(market_participant.market);
        market_stockpile += stockpile;
        // TODO(EhWhoAmI): Market prices
        stockpile.clear();
    }
}

void SysFactory::SysDemandResolver(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;

    // Process demand
    auto demand_processor_view = universe.view<cqspc::ResourceDemand, cqspc::MarketParticipant>();
    // Demand for next time
    for (auto [entity, demand, market_participant] : demand_processor_view.each()) {
        auto& market = universe.get<cqspc::Market>(market_participant.market);
        // Add to supply
        market.demand += demand;
        auto& market_stockpile = universe.get<cqspc::ResourceStockpile>(market_participant.market);

        // Check if it can handle it
        if (market_stockpile > demand) {
            universe.remove_if_exists<cqspc::FailedResourceTransfer>(entity);
        } else {
            universe.emplace_or_replace<cqspc::FailedResourceTransfer>(entity);
        }

        market_stockpile -= demand;
        if (universe.all_of<cqspc::ResourceStockpile>(entity)) {
            universe.get<cqspc::ResourceStockpile>(entity) += demand;
        }
        universe.remove<cqspc::ResourceDemand>(entity);
        // TODO(EhWhoAmI): Market prices
    }
}

void SysFactory::SysProductionStarter(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    // Gather resources for the next run, and signify if they want to produce
    auto production_view = universe.view<cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (auto [entity, converter, stockpile] : production_view.each()) {
        // Make the resources, and dump to market
        auto& recipe = universe.get<cqspc::Recipe>(converter.recipe);
        // resources generated:
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        // Wanted resources:
        if (stockpile >= recipe.input * productivity * Interval()) {
            stockpile -= recipe.input * productivity * Interval();
            // Produced, so remove the production
            universe.emplace<cqspc::Production>(entity);
        }
    }
}
