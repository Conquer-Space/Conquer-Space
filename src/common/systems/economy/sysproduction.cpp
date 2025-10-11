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
#include "common/systems/economy/sysproduction.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include <tracy/Tracy.hpp>

#include "common/components/area.h"
#include "common/components/market.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/surface.h"
#include "common/util/profiler.h"

namespace cqsp::common::systems {
/// <summary>
/// Runs the production cycle
/// Consumes material from the market based on supply and then sells the manufactured goods on the market.
/// </summary>
/// <param name="universe">Registry used for searching for components</param>
/// <param name="entity">Entity containing an Inudstries that need to be processed</param>
/// <param name="market">The market the industry uses.</param>
void ProcessIndustries(Node& node) {
    auto& universe = node.universe();
    auto& market = node.get<components::Market>();
    // Get the transport cost
    auto& infrastructure = node.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    auto& industries = node.get<components::IndustrialZone>();
    Node population_node = node.Convert(node.get<components::Settlement>().population.front());
    auto& population_wallet = population_node.get_or_emplace<components::Wallet>();
    for (Node industrynode : node.Convert(industries.industries)) {
        // Process imdustries
        // Industries MUST have production and a linked recipe
        if (!industrynode.all_of<components::Production>()) continue;
        Node recipenode = industrynode.Convert(industrynode.get<components::Production>().recipe);
        components::Recipe recipe = recipenode.get_or_emplace<components::Recipe>();
        components::IndustrySize& size = industrynode.get<components::IndustrySize>();
        // Calculate resource consumption
        components::ResourceLedger capitalinput = recipe.capitalcost * (size.size);
        components::ResourceLedger input = (recipe.input * size.utilization) + capitalinput;

        auto& employer = industrynode.get<components::Employer>();
        employer.population_fufilled = size.size * recipe.workers;

        // Calculate the greatest possible production
        components::ResourceLedger output;
        output[recipe.output.entity] = recipe.output.amount * size.utilization;

        // Figure out what's throttling production and maintenance
        // double limitedinput = CopyVals(input, market.history.back().sd_ratio).Min();
        // double limitedcapitalinput = CopyVals(capitalinput, market.history.back().sd_ratio).Min();

        // // Log how much manufacturing is being throttled by input
        // market[recipe.output.entity].inputratio = limitedinput;

        // if (market.sd_ratio[recipe.output.entity] < 1.1) {
        //     size.utilization *= 1 + (0.01) * std::fmin(limitedcapitalinput, 1);
        // } else {
        //     size.utilization *= 0.99;
        // }
        // size.utilization = std::clamp(size.utilization, 0., size.size);

        // Get the input goods and compare the

        market.consumption += input;
        market.production += output;

        double output_transport_cost = output.GetSum() * infra_cost;
        double input_transport_cost = input.GetSum() * infra_cost;
        // Next time need to compute the costs along with input and
        // output so that the factory doesn't overspend. We sorta
        // need a balanced economy
        components::CostBreakdown& costs = industrynode.get_or_emplace<components::CostBreakdown>();

        // Maintenance costs will still have to be upkept, so if
        // there isnt any resources to upkeep the place, then stop
        // the production
        costs.materialcosts = (input * market.price).GetSum();
        costs.wages = employer.population_fufilled * size.wages;
        costs.transport = 0;  //output_transport_cost + input_transport_cost;

        costs.revenue = (output * market.price).GetSum();
        costs.profit = costs.revenue - costs.maintenance - costs.materialcosts - costs.wages - costs.transport;

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
        bool shortage = false;
        double prod_sum = recipe.input.GetSum();
        for (auto& [good, amount] : recipe.input) {
            if (market.chronic_shortages[good] > 5) {
                // Reduce the amount based off the weighted average of the input?
                // Then reduce production over time or something
                shortage = true;
                break;
            }
        }

        double diff = 1 +
                      universe.economy_config.production_config.max_factory_delta /
                          (1 + std::exp(-(costs.profit * profit_multiplier))) -
                      universe.economy_config.production_config.max_factory_delta / 2;
        diff += universe.random->GetRandomNormal(0, 0.005);
        if (shortage) {
            diff -= std::max(universe.random->GetRandomNormal(0.1, 0.1), 0.02);
        }
        size.diff = diff;
        size.shortage = shortage;

        double past_util = size.utilization;
        size.utilization =
            std::clamp(size.utilization * diff,
                       universe.economy_config.production_config.factory_min_utilization * size.size, size.size);
        size.diff_delta = size.utilization - past_util;
        // Now diff it by that much
        // Let the minimum the factory can produce be like 10% of the
        // Pay the workers
        population_node.get<components::PopulationSegment>().income += costs.wages;
        population_node.get<components::PopulationSegment>().employed_amount += employer.population_fufilled;
        population_wallet += costs.wages;
        // If we have left over income we should improve the wages a little bit
        // There should also have a bank to reinvest into the company
        double pl_ratio = costs.profit / costs.revenue;
        if (pl_ratio > 0.1) {
            // Now we can expand it and improve our wages as well
            size.wages *= 1.05;
        } else if (pl_ratio < -0.1) {
            size.wages *= 0.95;
        }
    }
}

void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    // Each industrial zone is a a market
    BEGIN_TIMED_BLOCK(INDUSTRY);
    int factories = 0;
    // Loop through the markets
    int settlement_count = 0;
    // Get the markets and process the values?
    for (Node entity : universe.nodes<components::IndustrialZone, components::Market>()) {
        ProcessIndustries(entity);
    }
    END_TIMED_BLOCK(INDUSTRY);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}
}  // namespace cqsp::common::systems
