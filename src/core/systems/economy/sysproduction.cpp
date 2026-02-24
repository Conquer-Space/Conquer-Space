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
#include "core/systems/economy/sysproduction.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include <tracy/Tracy.hpp>

#include "core/components/area.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/surface.h"
#include "core/util/profiler.h"

namespace cqsp::core::systems {
void SysProduction::ScaleIndustry(Node& industry_node, components::Market& market) {
    Node recipenode = industry_node.Convert(industry_node.get<components::Production>().recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();
    components::IndustrySize& size = industry_node.get<components::IndustrySize>();
    auto& production_config = GetUniverse().economy_config.production_config;
    components::CostBreakdown& costs = industry_node.get_or_emplace<components::CostBreakdown>();
    auto& employer = industry_node.get<components::Employer>();

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

    // So we will add a random chance to increase or decrease profit
    // As we get more profit we should increase profit, as we get less profit we should reduce the size of the factory
    double diff =
        1 +
        production_config.max_factory_delta / (1 + std::exp(-(costs.profit * production_config.profit_multiplier))) -
        production_config.max_factory_delta / 2;
    diff += GetUniverse().random->GetRandomNormal(0, 0.001);
    if (shortage) {
        diff -= std::max(GetUniverse().random->GetRandomNormal(0.1, 0.1), 0.02);
    }
    size.diff = diff;
    size.shortage = shortage;

    int original_workers = employer.population_fufilled;
    double past_util = size.utilization;
    size.utilization =
        std::clamp(size.utilization * diff, production_config.factory_min_utilization * size.size, size.size);
    size.utilization = std::max(1., size.utilization);
    // Check if it's clamped and then check for the thing
    size.diff_delta = size.utilization - past_util;

    // Start hiring more when the utilization goes up
    double expected_workers = size.utilization * recipe.workers;
    employer.population_fufilled = static_cast<int>(expected_workers);

    // If we have left over income we should improve the wages a little bit
    // There should also have a bank to reinvest into the company
    double pl_ratio = costs.profit / costs.revenue;
    if (pl_ratio > 0.1) {
        // Now we can expand it and improve our wages as well
        size.wages *= 1.05;
    } else if (pl_ratio < -0.1) {
        size.wages *= 0.95;
    }
    double utilization_ratio = size.utilization / size.size;
    // Now we should compute if we are constructing or not and if we aren't then we should reduce if the ratio is below
    // a certain ratio
    double factor = (0.4 - utilization_ratio);
    if (factor < 0) {
        factor = 0;
        size.underutilization = 0;
    }
    // Normalize
    factor /= 0.4;
    size.underutilization += factor;

    if (size.underutilization > production_config.underutilization_limit &&
        !industry_node.all_of<components::Construction>()) {
        // Then we should shrink the size of the factory by a certain factor since we have been not using the factory
        size.size *= 0.9;
        // Reset our underutilization so that we don't immediately kill our production
        size.underutilization = 0;
    }

    if (pl_ratio > 0.25 && size.continuous_gains > production_config.construction_limit &&
        size.utilization >= size.size && !industry_node.all_of<components::Construction>()) {
        // what's the ratio we should expand the factory at lol
        // Now we should expand it...
        // pl_ratio should be maybe
        // Set our construction costs
        if (recipenode.all_of<components::ConstructionCost>()) {
            const auto& construction_cost = recipenode.get<components::ConstructionCost>();
            auto& construction =
                industry_node.emplace<components::Construction>(0, 20, static_cast<int>(0.25 * size.size * pl_ratio));

        } else {
            auto& construction =
                industry_node.emplace<components::Construction>(0, 20, static_cast<int>(0.25 * size.size * pl_ratio));
        }
    }

    // Let's start laying off people too if we have too much of a cut
    if (size.continuous_losses > 5) {
        employer.population_fufilled *= 0.95;
        // Also scale the scale with this
        double workers_count = std::floor(employer.population_fufilled / recipe.workers);
        size.utilization = std::min(workers_count, size.utilization);
        // We should still minimize
        size.utilization = std::max(1., size.utilization);
    }

    if (size.continuous_gains > 5) {
        employer.population_fufilled *= 1.05;
        // Also scale the scale with this
        double workers_count = std::floor(employer.population_fufilled / recipe.workers);
        size.utilization = std::min(workers_count, size.utilization);
        // We should still minimize
        size.utilization = std::max(1., size.utilization);
    }
    if (costs.profit < 0) {
        size.continuous_losses++;
    } else {
        size.continuous_losses = 0;
    }
    if (costs.profit > 0) {
        size.continuous_gains++;
    } else {
        size.continuous_gains = 0;
    }
    employer.population_change = original_workers - employer.population_fufilled;
}

void SysProduction::ProcessIndustry(Node& industry_node, components::Market& market, Node& population_node,
                                    double infra_cost) {
    ZoneScoped;
    if (industry_node.any_of<components::Construction>()) {
        // Then progress construction
        auto& construction_progress = industry_node.get<components::Construction>();
        // Process construction costs
        construction_progress.progress++;
        int size = construction_progress.levels;
        if (construction_progress.progress >= construction_progress.maximum) {
            industry_node.get<components::IndustrySize>().size += construction_progress.levels;
            industry_node.remove<components::Construction>();
        }
        if (size == 0) {
            return;
        }
    }
    auto& production_config = GetUniverse().economy_config.production_config;
    auto& population_wallet = population_node.get_or_emplace<components::Wallet>();
    auto& employer = industry_node.get<components::Employer>();
    components::CostBreakdown& costs = industry_node.get_or_emplace<components::CostBreakdown>();

    // Process imdustries
    // Industries MUST have production and a linked recipe
    if (!industry_node.all_of<components::Production>()) return;
    ScaleIndustry(industry_node, market);

    Node recipenode = industry_node.Convert(industry_node.get<components::Production>().recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();
    components::IndustrySize& size = industry_node.get<components::IndustrySize>();

    // Let's calculate the size from previous input
    // Calculate resource consumption
    components::ResourceMap capitalinput = recipe.capitalcost * (size.size);
    components::ResourceMap input = (recipe.input * size.utilization) + capitalinput;

    // Calculate the greatest possible production
    components::ResourceMap output;
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
    costs.amount_sold = recipe.output.amount * size.utilization;

    double output_transport_cost = output.GetSum() * infra_cost;
    double input_transport_cost = input.GetSum() * infra_cost;
    // Next time need to compute the costs along with input and
    // output so that the factory doesn't overspend. We sorta
    // need a balanced economy

    // Maintenance costs will still have to be upkept, so if
    // there isnt any resources to upkeep the place, then stop
    // the production
    costs.material_costs = (input * market.price).GetSum();
    costs.wages = employer.population_fufilled * size.wages;
    costs.transport = 0;  //output_transport_cost + input_transport_cost;

    costs.revenue = (output * market.price).GetSum();
    costs.profit = costs.revenue - costs.maintenance - costs.material_costs - costs.wages - costs.transport;
    auto& wallet = industry_node.get<components::Wallet>();
    wallet += costs.profit;
    /*
        Now try to maximize profit
        Maximizing profit is a two fold thing
        If the S/D ratio is < 1, and the profit is negative, that means that this factory is not supplying enough
        This could be either the good that we are producing is not profitable, or the fact that our base costs
        are too high.

        For the former option, we should reduce production because our goods are not making a profit and cut costs
        until we can make a profit, either by squeezing out the market. If we're not big enough to change the market
        we will just go out of business.
        and for the second production, we should increase our production because we just need more production so
        that we can get to profitability

        If S/D ratio is > 1, and we are still making negative profit, we are producing too much, or paying the workers
        too much. There are so many knobs that we have to tune, so I'm not sure how we can simplify this into a few
        simple knobs (more like one)

        I think one of the issues that we have is what if all the businesses go out of business at one time, and end
        up just killing off the specific market for a good?
        Do we need to prop things out
        or have a stage where it stays hibernated for a while, and then ramps up production if it can become profitable

        Right now we should naively modify the price then to maximize profit
        Now let's target profit only
        If we're making more money, increase utilization, if we're making less money, reduce utilization
        We can only reduce and increase production by a certain amount, let's say a maximum of 5%

        Then the other thing is in that case, it would just have boom and busts
        If we make the time that a business dies random, then perhaps we could tune it more
        Now what's the goal
        The more profit we have the less we increase until some level
        Let's just make it a log level
        */

    // Now diff it by that much
    // Let the minimum the factory can produce be like 10% of the
    // Pay the workers
    auto& population_segment = population_node.get<components::PopulationSegment>();
    population_segment.income += costs.wages;
    population_segment.employed_amount += employer.population_fufilled;
    population_wallet += costs.wages;
}
/// <summary>
/// Runs the production cycle
/// Consumes material from the market based on supply and then sells the manufactured goods on the market.
/// </summary>
/// <param name="universe">Registry used for searching for components</param>
/// <param name="entity">Entity containing an Inudstries that need to be processed</param>
/// <param name="market">The market the industry uses.</param>
void SysProduction::ProcessIndustries(Node& node) {
    ZoneScoped;
    auto& market = node.get<components::Market>();
    auto& production_config = GetUniverse().economy_config.production_config;
    // Get the transport cost
    auto& infrastructure = node.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    auto& industries = node.get<components::IndustrialZone>();
    if (node.get<components::Settlement>().population.empty()) {
        return;
    }
    Node population_node = node.Convert(node.get<components::Settlement>().population.front());
    for (Node industry_node : node.Convert(industries.industries)) {
        // We should also check for industries we want to construct
        ProcessIndustry(industry_node, market, population_node, infra_cost);
    }
}

void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    // Each industrial zone is a a market
    BEGIN_TIMED_BLOCK(Industry);
    int factories = 0;
    // Loop through the markets
    int settlement_count = 0;
    // Get the markets and process the values?
    for (Node entity : universe.nodes<components::IndustrialZone, components::Market>()) {
        ProcessIndustries(entity);
    }
    END_TIMED_BLOCK(Industry);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}
}  // namespace cqsp::core::systems
