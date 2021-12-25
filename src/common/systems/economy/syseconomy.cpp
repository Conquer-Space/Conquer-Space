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
#include "common/components/population.h"
#include "common/components/surface.h"
#include "common/systems/economy/sysmarket.h"

#include "common/util/profiler.h"

// Economy is kinda weird, and we probably have to overhaul it. Here are some ideas for an overhaul:
// TODO(EhWhoAmI): Remove resource stockpiles from factories and mines and dump resources directly on
// the market
// TODO(EhWhoAmI): Unify resource generation and consumption into one ledger that we can create and write to
// we already have demand and supply, but I think it could be a bit more rigid, whatever that means.

namespace cqsp::common::systems {
void SysWalletReset::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Wallet>();
    for (entt::entity entity : view) {
        GetUniverse().get<cqspc::Wallet>(entity).Reset();
    }
}

void SysCommercialProcess::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Commercial>();
    for (entt::entity entity : view) {
        // If it's a commercial area, add employing things
        // They will employ from a place by default
        auto& employer = GetUniverse().get<cqspc::Employer>(entity);
        // Get population, presumably it's a city, so we would know how many
        // people we have Get city population
        //
        // For now, services will just fall under general services, but a more
        // detailed breakdown will be done in the future.

        auto& commercial = GetUniverse().get<cqspc::Commercial>(entity);
        // Get population
        entt::entity population_segment =
            GetUniverse().get<cqspc::Settlement>(commercial.city).population.front();
        employer.population_needed =
            GetUniverse().get<cqspc::PopulationSegment>(population_segment).population * 0.5;
    }
}

void SysEmploymentHandler::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Settlement, cqspc::Industry>();
    SPDLOG_TRACE("Organizing jobs for {} settlements", view.size_hint());
    for (auto entity : view) {
        // Now iterate through the population segments, and the industrial
        // things, and determine the number of jobs needed. Because we only have
        // one population segment for each city, we would only take from the
        // first segment
        // TODO(EhWhoAmI): Take into account multiple population segments
        auto& jib = GetUniverse().get<cqspc::Employee>(
            GetUniverse().get<cqspc::Settlement>(entity).population[0]);
        jib.employed_population = 0;

        for (auto factory : GetUniverse().get<cqspc::Industry>(entity).industries) {
            // Get all the available jobs
            if (GetUniverse().all_of<cqspc::Employer>(factory)) {
                // Then assign the work
                auto& jobs = GetUniverse().get<cqspc::Employer>(factory);
                jib.employed_population += jobs.population_needed;
            }
        }
    }
}

void SysResourceGenerator::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view =
        GetUniverse().view<cqspc::ResourceGenerator, cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Creating resources for {} resource generators",
                 view.size_hint());
    for (auto entity : view) {
        auto& production = GetUniverse().get<cqspc::ResourceGenerator>(entity);
        auto& stockpile = GetUniverse().get<cqspc::ResourceStockpile>(entity);
        // Make the resources, and dump to market
        // resources generated:
        float productivity = 1;
        if (GetUniverse().all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = GetUniverse().get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile.MultiplyAdd(production, productivity * Interval());
    }
}

void SysProduction::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Production, cqspc::ResourceConverter,
                              cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Creating resources for {} factories", view.size_hint());
    for (auto [entity, production, converter, stockpile] : view.each()) {
        // Make the resources, and dump to market
        auto& recipe = GetUniverse().get<cqspc::Recipe>(converter.recipe);
        // resources generated:
        float productivity = 1;
        if (GetUniverse().all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = GetUniverse().get<cqspc::FactoryProductivity>(entity).productivity;
        }
        stockpile.MultiplyAdd(recipe.output, productivity * Interval());
        // Produced, so remove the production
        GetUniverse().remove<cqspc::Production>(entity);
    }
}

void SysDemandCreator::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::ResourceConsumption, cqspc::MarketAgent>();
    SPDLOG_TRACE("Creating {} resource consumption demands", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& consumption = GetUniverse().get<cqspc::ResourceConsumption>(entity);
        auto& participant = GetUniverse().get<cqspc::MarketAgent>(entity);
        float productivity = 1;
        if (GetUniverse().all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = GetUniverse().get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto& demand = GetUniverse().emplace_or_replace<cqspc::ResourceDemand>(entity);
        demand.MultiplyAdd(consumption, productivity * Interval());
    }
}

void SysFactoryDemandCreator::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::ResourceConverter, cqspc::MarketAgent>();
    SPDLOG_TRACE("Creating {} factory demands", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& converter = GetUniverse().get<cqspc::ResourceConverter>(entity);
        auto& participant = GetUniverse().get<cqspc::MarketAgent>(entity);
        auto& recipe = GetUniverse().get<cqspc::Recipe>(converter.recipe);

        float productivity = 1;
        if (GetUniverse().all_of<cqspc::FactoryProductivity>(entity)) {
            productivity =
                GetUniverse().get<cqspc::FactoryProductivity>(entity).productivity;
        }
        auto& demand = GetUniverse().emplace<cqspc::ResourceDemand>(entity);
        demand.MultiplyAdd(recipe.input, productivity * Interval());
    }
}

void SysGoodSeller::DoSystem() {
    namespace cqspc = cqsp::common::components;

    // Sell all goods
    auto view = GetUniverse().view<cqspc::ResourceStockpile, cqspc::MarketAgent>();
    SPDLOG_TRACE("Selling for {} stockpiles", view.size_hint());
    // Demand for next time
    for (auto [entity, stockpile, market_participant] : view.each()) {
        // auto& market =
        // GetUniverse().get<cqspc::Market>(market_participant.market);
        // Add to supply
        auto& market_stockpile =
            GetUniverse().get<cqspc::ResourceStockpile>(market_participant.market);
        market_stockpile += stockpile;
        auto& market = GetUniverse().get<cqspc::Market>(market_participant.market);
        // Sell goods
        // Adjust wallet
        GetUniverse().get<cqspc::Wallet>(entity) +=
            stockpile.MultiplyAndGetSum(market.prices);
        stockpile.clear();
    }
}

void SysPriceDetermine::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Market>();
    for (entt::entity entity : view) {
        // Sort through market, and get goods
        auto& market = GetUniverse().get<cqspc::Market>(entity);
        // Sort through demand and calculate things
        cqsp::common::systems::market::DeterminePrices(GetUniverse(), entity);
        // Clear demand because we have already determined all the things we
        // needed
        market.demand.clear();
    }
}

void cqsp::common::systems::SysConsumptionConsume::DoSystem() {
    namespace cqspc = cqsp::common::components;

    auto view =
        GetUniverse().view<cqspc::ResourceConsumption, cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Resolving consumption for {} entities", view.size_hint());
    for (entt::entity entity : view) {
        auto& stockpile = GetUniverse().get<cqspc::ResourceStockpile>(entity);
        auto& consumption = GetUniverse().get<cqspc::ResourceConsumption>(entity);
        // FIXME(EhWhoAmI): This leaves resources at negative level, and I don't
        // know why Good news is it doesn't go down lower than a certain amount,
        // so maybe it's a problem with the order it's doing it
        if (stockpile.EnoughToTransfer(consumption)) {
            stockpile.MultiplyAdd(consumption, Interval() * -1);
        } else {
            // Clear the resources in it.
            // Also add a tag that it has not enough resources
            stockpile.RemoveResourcesLimited(consumption);
        }
    }
}

void SysDemandResolver::DoSystem() {
    namespace cqspc = cqsp::common::components;

    // Process demand
    auto view = GetUniverse().view<cqspc::ResourceDemand, cqspc::MarketAgent,
                              cqspc::ResourceStockpile>();
    SPDLOG_TRACE("Resolving demand for {} participants", view.size_hint());
    // Demand for next time
    for (auto entity : view) {
        auto& market_participant = GetUniverse().get<cqspc::MarketAgent>(entity);
        auto& demand = GetUniverse().get<cqspc::ResourceDemand>(entity);
        auto& market = GetUniverse().get<cqspc::Market>(market_participant.market);
        // Add to demand
        market.demand += demand;
        auto& market_stockpile =
            GetUniverse().get<cqspc::ResourceStockpile>(market_participant.market);

        // Check if it can handle it, and transfer resources on success
        if (market_stockpile.EnoughToTransfer(demand)) {
            GetUniverse().remove_if_exists<cqspc::FailedResourceTransfer>(entity);
            // Market prices
            // TODO(EhWhoAmI): Fix wallet so that it takes into account negative
            // values better, So that it would buy less stuff when it has a low
            // wallet balance Buy all the resources
            cqspc::Wallet& balance = GetUniverse().get<cqspc::Wallet>(entity);
            double cost = demand.MultiplyAndGetSum(market.prices);
            if (cost > balance) {
                // Failed transaction
                // GetUniverse().emplace_or_replace<cqspc::FailedResourceTransfer>(entity);
                // Try to buy the maximum available
                // Get ratio of the things, multiply by price, and then
                demand *= (balance / cost);
                cost = demand.MultiplyAndGetSum(market.prices);
                balance = 0;
                market_stockpile.TransferTo(
                    GetUniverse().get<cqspc::ResourceStockpile>(entity), demand);
            } else {
                balance -= demand.MultiplyAndGetSum(market.prices);
                market_stockpile.TransferTo(
                    GetUniverse().get<cqspc::ResourceStockpile>(entity), demand);
            }
        } else {
            // Failed due to not enough resources in the market,
            // TODO(EhWhoAmI): Try to get all the resources remaining in the
            // stockpile
            GetUniverse().emplace_or_replace<cqspc::FailedResourceTransfer>(entity);
        }

        GetUniverse().remove<cqspc::ResourceDemand>(entity);
    }
}

void SysProductionStarter::DoSystem() {
    namespace cqspc = cqsp::common::components;
    // Gather resources for the next run, and signify if they want to produce
    auto production_view = GetUniverse().view<cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (auto [entity, converter, stockpile] : production_view.each()) {
        // Make the resources, and dump to market
        auto& recipe = GetUniverse().get<cqspc::Recipe>(converter.recipe);
        // resources generated:
        float productivity = 1;
        if (GetUniverse().all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = GetUniverse().get<cqspc::FactoryProductivity>(entity).productivity;
        }
        // Check if there are enough people working
        if (GetUniverse().all_of<cqspc::Employer>(entity)) {
            auto& employer = GetUniverse().get<cqspc::Employer>(entity);
            if (employer.population_needed > employer.population_fufilled) {
                // Then not enough people, and then it cannot work
                GetUniverse().emplace_or_replace<cqspc::FailedResourceProduction>(entity);
                continue;
            }
        }

        // Wanted resources:
        cqspc::ResourceLedger stockpile_calc;
        stockpile_calc.MultiplyAdd(recipe.input, productivity * Interval());
        // Also verify if there were enough people working there to ensure
        // that the area is working
        if (stockpile.EnoughToTransfer(stockpile_calc)) {
            stockpile -= stockpile_calc;
            // Produced, so add production
            GetUniverse().emplace<cqspc::Production>(entity);
            GetUniverse().remove_if_exists<cqspc::FailedResourceProduction>(entity);
        } else {
            // Or else notify people that nothing was produced
            GetUniverse().emplace_or_replace<cqspc::FailedResourceProduction>(entity);
        }
    }
}

void SysMarketResolver::DoSystem() {
    // Balances economy production and supply
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::ResourceGenerator, cqspc::MarketAgent>();
    for (entt::entity entity : view) {
        // Sort through the products of the demand, and cut the supply to zero if the s/d ratio is too high
        auto& generator = GetUniverse().get<cqspc::ResourceGenerator>(entity);
        auto& agent = GetUniverse().get<cqspc::MarketAgent>(entity);
        auto& market = GetUniverse().get<cqspc::Market>(agent.market);
        for (auto e : generator) {
            double sd_ratio = market.sd_ratio[e.first];
            // Reduce supply if it's too high
        }
    }
}

}  // namespace cqsp::common::systems
