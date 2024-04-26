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
#include "common/systems/economy/sysfactory.h"

#include <spdlog/spdlog.h>

#include <tracy/Tracy.hpp>

#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/surface.h"
#include "common/util/profiler.h"

namespace components = cqsp::common::components;
using components::ResourceLedger;
using components::IndustrySize;
using components::Recipe;
using components::Production;
using entt::entity;

namespace cqsp::common::systems {
/// <summary>
/// Runs the production cycle
/// Consumes material from the market based on supply and then sells the manufactured goods on the market.
/// </summary>
/// <param name="universe">Registry used for searching for components</param>
/// <param name="entity">Entity containing an Inudstries that need to be processed</param>
/// <param name="market">The market the industry uses.</param>
void ProcessIndustries(Universe& universe, entity entity) {
    auto& market = universe.get<components::Market>(entity);
    // Get the transport cost
    auto& infrastructure = universe.get<components::infrastructure::CityInfrastructure>(entity);
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    auto& population_wallet =
        universe.get_or_emplace<components::Wallet>(universe.get<components::Settlement>(entity).population.front());
    for (entt::entity productionentity : universe.get<components::IndustrialZone>(entity).industries) {
        // Process imdustries
        // Industries MUST have production and a linked recipe
        if (!universe.all_of<Production>(productionentity)) continue;
        Recipe recipe = universe.get_or_emplace<Recipe>(universe.get<Production>(productionentity).recipe);
        IndustrySize& size = universe.get_or_emplace<IndustrySize>(productionentity, 1000.0);
        // Calculate resource consumption
        ResourceLedger capitalinput = recipe.capitalcost * (0.01 * size.size);
        ResourceLedger input = (recipe.input + size.utilization) + capitalinput;

        // Calculate the greatest possible production
        ResourceLedger output;  // * ratio.output;
        output[recipe.output.entity] = recipe.output.amount * size.utilization;

        // Figure out what's throttling production and maintenance
        double limitedinput = CopyVals(input, market.history.back().sd_ratio).Min();
        double limitedcapitalinput = CopyVals(capitalinput, market.history.back().sd_ratio).Min();

        // Log how much manufacturing is being throttled by input
        market[recipe.output.entity].inputratio = limitedinput;

        if (market.sd_ratio[recipe.output.entity] < 1.1) {
            size.utilization *= 1 + (0.01) * std::fmin(limitedcapitalinput, 1);
        } else {
            size.utilization *= 0.99;
        }
        size.utilization = std::clamp(size.utilization, 0., size.size);

        if (limitedinput < 1) {  // If an input good is undersupplied on
                                 // the market, throttle production
            input *= limitedinput;
            output *= limitedinput;
            // Industry
        }

        market.demand += input;
        market.supply += output;

        double output_transport_cost = output.GetSum() * infra_cost;
        double input_transport_cost = input.GetSum() * infra_cost;
        // Next time need to compute the costs along with input and
        // output so that the factory doesn't overspend. We sorta
        // need a balanced economy
        components::CostBreakdown& costs = universe.get_or_emplace<components::CostBreakdown>(productionentity);

        // Maintenance costs will still have to be upkept, so if
        // there isnt any resources to upkeep the place, then stop
        // the production
        costs.materialcosts = (recipe.input * size.utilization * market.price).GetSum();
        costs.revenue = (recipe.output * market.price).GetSum();
        if (market.sd_ratio[recipe.output.entity] > 1) {
            costs.revenue /= market.sd_ratio[recipe.output.entity];
        }
        costs.wages = size.size * recipe.workers * size.wages;
        costs.profit = costs.revenue - costs.maintenance - costs.materialcosts - costs.wages;
        costs.transport = output_transport_cost + input_transport_cost;
        double& price = market.price[recipe.output.entity];
        if (costs.profit > 0) {
            price += (-0.1 + price * -0.01f);
        } else {
            price += (0.2 + price * 0.01f);
        }

        // Pay the workers
        population_wallet += costs.wages;
    }
}
void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    auto view = universe.view<components::IndustrialZone, components::Market>();
    // Each industrial zone is a a market
    BEGIN_TIMED_BLOCK(INDUSTRY);
    int factories = 0;
    // Loop through the markets
    int settlement_count = 0;
    // Get the markets and process the values?
    for (entity entity : view) {
        ProcessIndustries(universe, entity);
    }
    END_TIMED_BLOCK(INDUSTRY);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}
}  // namespace cqsp::common::systems
