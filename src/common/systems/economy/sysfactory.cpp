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
#include "common/systems/economy/sysfactory.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include <tracy/Tracy.hpp>

#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/surface.h"
#include "common/util/profiler.h"

namespace cqsp::common::systems {
namespace cqspc = cqsp::common::components;
namespace {
/// <summary>
/// Runs the production cycle
/// Consumes material from the market based on supply and then sells the manufactured goods on the market.
/// </summary>
/// <param name="universe">Registry used for searching for components</param>
/// <param name="entity">Entity containing an Inudstries that need to be processed</param>
/// <param name="market">The market the industry uses.</param>
void ProcessIndustries(Universe& universe, entt::entity entity) {
    auto& market = universe.get<components::Market>(entity);
    // Get the transport cost
    auto& infrastructure = universe.get<cqspc::infrastructure::CityInfrastructure>(entity);
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    auto& industries = universe.get<cqspc::IndustrialZone>(entity);
    auto& population_wallet =
        universe.get_or_emplace<cqspc::Wallet>(universe.get<cqspc::Settlement>(entity).population.front());
    for (entt::entity productionentity : industries.industries) {
        // Process imdustries
        // Industries MUST have production and a linked recipe
        if (!universe.all_of<components::Production>(productionentity)) continue;
        components::Recipe recipe =
            universe.get_or_emplace<components::Recipe>(universe.get<components::Production>(productionentity).recipe);
        components::IndustrySize& size = universe.get<components::IndustrySize>(productionentity);
        // Calculate resource consumption
        components::ResourceLedger capitalinput = recipe.capitalcost * (size.size);
        components::ResourceLedger input = (recipe.input * size.utilization) + capitalinput;

        auto& employer = universe.get<components::Employer>(productionentity);
        employer.population_fufilled = size.size * recipe.workers;

        // Calculate the greatest possible production
        components::ResourceLedger output;
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

        market.consumption += input;
        market.production += output;

        double output_transport_cost = output.GetSum() * infra_cost;
        double input_transport_cost = input.GetSum() * infra_cost;
        // Next time need to compute the costs along with input and
        // output so that the factory doesn't overspend. We sorta
        // need a balanced economy
        components::CostBreakdown& costs = universe.get_or_emplace<components::CostBreakdown>(productionentity);

        // Maintenance costs will still have to be upkept, so if
        // there isnt any resources to upkeep the place, then stop
        // the production
        costs.materialcosts = (input * market.price).GetSum();
        costs.wages = size.size * recipe.workers * size.wages;

        costs.revenue = (recipe.output * market.price).GetSum();
        costs.profit = costs.revenue - costs.maintenance - costs.materialcosts - costs.wages;
        costs.transport = output_transport_cost + input_transport_cost;

        // Now try to maximize profit
        // Maximizing profit is a two fold thing
        // If the S/D ratio is < 1, and the profit is negative, that means that this factory is not supplying enough
        // This could be either the good that we are producing is not profitable, or the fact that our base costs
        // are too high.

        // For the former option, we should reduce production because our goods are not making a profit and cut costs
        // until we can make a profit, either by squeezing out the market. If we're not big enough to change the market
        // we will just go out of business.
        // and for the second production, we should increase our production because we just need more production so
        // that we can get to profitability

        // If S/D ratio is > 1, and we are still making negative profit, we are producing too much, or paying the workers
        // too much. There are so many knobs that we have to tune, so I'm not sure how we can simplify this into a few
        // simple knobs (more like one)

        // I think one of the issues that we have is what if all the businesses go out of business at one time, and end
        // up just killing off the specific market for a good?
        // Do we need to prop things out
        // or have a stage where it stays hibernated for a while, and then ramps up production if it can become profitable

        // Right now we should naively modify the price then to maximize profit
        // Now let's target profit only
        // If we're making more money, increase utilization, if we're making less money, reduce utilization
        // We can only reduce and increase production by a certain amount, let's say a maximum of 5%

        // Then the other thing is in that case, it would just have boom and busts
        // If we make the time that a business dies random, then perhaps we could tune it more
        // Now what's the goal
        // The more profit we have the less we increase until some level
        // Let's just make it a log level
        // TODO(EhWhoAmI): This should just be some sort of setting tbh
        float profit_multiplier = 0.001;
        // but if we have close to zero profit, we want to take risks and move in a certain direction.

        // So we will add a random chance to increase or decrease profit
        double diff = std::clamp(log(fabs(costs.profit) * profit_multiplier), 0., 0.05);
        diff += 1 + universe.random->GetRandomNormal(0, 0.075);
        diff *= (costs.profit < 0) ? -1 : 1;
        size.utilization = std::clamp(size.utilization * diff, 0.1 * size.size, size.size);
        // Now diff it by that much
        // Let the minimum the factory can produce be like 10% of the
        // Pay the workers
        population_wallet += costs.wages;
    }
}
}  // namespace

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
    for (entt::entity entity : view) {
        ProcessIndustries(universe, entity);
    }
    END_TIMED_BLOCK(INDUSTRY);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}
}  // namespace cqsp::common::systems
