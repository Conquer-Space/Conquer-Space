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
#include "common/systems/economy/syseconomy.h"

#include <spdlog/spdlog.h>

#include "common/components/resource.h"
#include "common/components/economy.h"
#include "common/components/area.h"

#include "common/util/profiler.h"

//
// Economy is kinda weird, and we probably have to overhaul it. Here are some ideas for an overhaul:
// TODO(EhWhoAmI): Remove resource stockpiles from factories and mines and dump resources directly on
// the market
// TODO(EhWhoAmI): Unify resource generation and consumption into one ledger that we can create and write to
// we already have demand and supply, but I think it could be a bit more rigid, whatever that means.
//

using cqsp::common::systems::SysFactory;
void SysFactory::DoSystem(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    BEGIN_TIMED_BLOCK(SysFactory);
    // Produce resources
    BEGIN_TIMED_BLOCK(Resource_gen);
    SysResourceGenerator(universe);
    SysProduction(universe);
    SysDemand(universe);
    SysFactoryDemandCreator(universe);
    END_TIMED_BLOCK(Resource_gen);
    BEGIN_TIMED_BLOCK(Market_sim);
    SysGoodSeller(universe);
    SysDemandResolver(universe);
    END_TIMED_BLOCK(Market_sim);
    BEGIN_TIMED_BLOCK(Production_sim);
    SysProductionStarter(universe);
    END_TIMED_BLOCK(Production_sim);
    END_TIMED_BLOCK(SysFactory);
}

void SysFactory::SysResourceGenerator(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    auto view = universe.view<cqspc::ResourceGenerator, cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Creating resources for {} resource generators", view.size_hint());
    for (auto entity : view) {
        auto& production = universe.get<cqspc::ResourceGenerator>(entity);
        auto& stockpile = universe.get<cqspc::ResourceStockpile>(entity);
        // Make the resources, and dump to market
        // resources generated:
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile.MultiplyAdd(production, productivity * Interval());
    }
}

void SysFactory::SysProduction(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    auto view = universe.view<cqspc::Production, cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Creating resources for {} factories", view.size_hint());
    for (auto [entity, production, converter, stockpile] : view.each()) {
        // Make the resources, and dump to market
        auto& recipe = universe.get<cqspc::Recipe>(converter.recipe);
        // resources generated:
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile.MultiplyAdd(recipe.output, productivity * Interval());
        // Produced, so remove the production
        universe.remove<cqspc::Production>(entity);
    }
}

void SysFactory::SysDemand(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    auto view = universe.view<cqspc::ResourceConsumption, cqspc::MarketParticipant>();
    SPDLOG_TRACE("Creating {} resource consumption demands", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& consumption = universe.get<cqspc::ResourceConsumption>(entity);
        auto& participant = universe.get<cqspc::MarketParticipant>(entity);
        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto& demand = universe.emplace_or_replace<cqspc::ResourceDemand>(entity);
        demand.MultiplyAdd(consumption, productivity * Interval());
    }
}

void SysFactory::SysFactoryDemandCreator(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    auto view = universe.view<cqspc::ResourceConverter, cqspc::MarketParticipant>();
    SPDLOG_TRACE("Creating {} factory demands", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& converter = universe.get<cqspc::ResourceConverter>(entity);
        auto& participant = universe.get<cqspc::MarketParticipant>(entity);
        auto& recipe = universe.get<cqspc::Recipe>(converter.recipe);

        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto &demand = universe.emplace<cqspc::ResourceDemand>(entity);
        demand.MultiplyAdd(recipe.input, productivity * Interval());
    }
}

void SysFactory::SysGoodSeller(Universe& universe) {
    namespace cqspc = cqsp::common::components;

    // Sell all goods
    auto view = universe.view<cqspc::ResourceStockpile, cqspc::MarketParticipant>();
    SPDLOG_TRACE("Selling for {} stockpiles", view.size_hint());
    // Demand for next time
    for (auto [entity, stockpile, market_participant] : view.each()) {
        //auto& market = universe.get<cqspc::Market>(market_participant.market);
        // Add to supply
        auto& market_stockpile = universe.get<cqspc::ResourceStockpile>(market_participant.market);
        market_stockpile += stockpile;
        // TODO(EhWhoAmI): Market prices
        stockpile.clear();
    }
}

void SysFactory::SysDemandResolver(Universe& universe) {
    namespace cqspc = cqsp::common::components;

    // Process demand
    auto view = universe.view<cqspc::ResourceDemand, cqspc::MarketParticipant, cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Resolving demand for {} participants", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& market_participant = universe.get<cqspc::MarketParticipant>(entity);
        auto& demand = universe.get<cqspc::ResourceDemand>(entity);
        auto& market = universe.get<cqspc::Market>(market_participant.market);
        // Add to demand
        market.demand += demand;
        auto& market_stockpile = universe.get<cqspc::ResourceStockpile>(market_participant.market);

        // Check if it can handle it, and transfer resources on success
        if (market_stockpile > demand) {
            universe.remove_if_exists<cqspc::FailedResourceTransfer>(entity);
            if (universe.all_of<cqspc::ResourceStockpile>(entity)) {
                market_stockpile.TransferTo(universe.get<cqspc::ResourceStockpile>(entity), demand);
            }
        } else {
            universe.emplace_or_replace<cqspc::FailedResourceTransfer>(entity);
        }

        universe.remove<cqspc::ResourceDemand>(entity);
        // TODO(EhWhoAmI): Market prices
    }
}

void SysFactory::SysProductionStarter(Universe& universe) {
    namespace cqspc = cqsp::common::components;
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
        cqspc::ResourceLedger stockpile_calc;
        stockpile_calc.MultiplyAdd(recipe.input, productivity * Interval());
        if (stockpile.EnoughToTransfer(stockpile_calc)) {
            stockpile -= stockpile_calc;
            // Produced, so add production
            universe.emplace<cqspc::Production>(entity);
            universe.remove_if_exists<cqspc::FailedResourceProduction>(entity);
        } else {
            // Or else notify people that nothing was produced
            universe.emplace_or_replace<cqspc::FailedResourceProduction>(entity);
        }
    }
}