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
#include "core/components/history.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
void SysProduction::ProcessIndustry(Node& industry_node, components::Market& market, Node& population_node,
                                    double infra_cost) {
    ZoneScoped;
    auto& production_config = GetUniverse().economy_config.production_config;
    auto& population_wallet = population_node.get_or_emplace<components::Wallet>();
    auto& employer = industry_node.get<components::Employer>();
    auto& population_segment = population_node.get<components::PopulationSegment>();

    // Process imdustries
    // Industries MUST have production and a linked recipe
    if (!industry_node.all_of<components::ProductionUnit>()) return;

    components::ProductionUnit& size = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(size.recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();
    double expected_workers = size.utilization * recipe.workers;
    employer.population_fufilled = static_cast<int>(expected_workers);

    // Let's calculate the size from previous input
    // Calculate resource consumption
    components::ResourceVector capitalinput = recipe.capitalcost * (size.size);
    components::ResourceVector input = (recipe.input * size.utilization) + capitalinput;

    // Calculate the greatest possible production
    components::ResourceVector output;
    output.push_back(std::pair(recipe.output.entity, recipe.output.amount * size.utilization));

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

    // We should also drift wages towards the average wage of the pop node or something
    double delta = size.wages * size.ProfitMargin() * 0.1;
    delta = std::clamp(delta, -size.wages * 0.05, size.wages * 0.05);
    size.wages += delta;
    // Also pull our wage to the average
    size.wages = size.wages * 0.9 + population_segment.average_wage * 0.1;
    size.wages = std::max(1.0, size.wages);
    market.consumption += input;
    market.production += output;
    size.amount_sold = recipe.output.amount * size.utilization;

    double output_transport_cost = output.GetSum() * infra_cost;
    double input_transport_cost = input.GetSum() * infra_cost;
    // Next time need to compute the costs along with input and
    // output so that the factory doesn't overspend. We sorta
    // need a balanced economy

    // Maintenance costs will still have to be upkept, so if
    // there isnt any resources to upkeep the place, then stop
    // the production
    size.material_costs = input.MultiplyAndGetSum(market.price);
    size.wage_cost = employer.population_fufilled * size.wages;
    size.transport = 0;  //output_transport_cost + input_transport_cost;

    size.revenue = output.MultiplyAndGetSum(market.price);
    size.profit = size.revenue - size.maintenance - size.material_costs - size.wage_cost - size.transport;
    auto& wallet = industry_node.get<components::Wallet>();
    wallet += size.profit;
    market.GDP += size.revenue - size.material_costs;
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

    population_segment.income += size.wage_cost;
    population_segment.employed_amount += employer.population_fufilled;
    population_wallet += size.wage_cost;
}

void SysProduction::ScaleConstruction(Node& industry_node, double pl_ratio) {
    ZoneScoped;
    components::ProductionUnit& production_unit = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(production_unit.recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();
    const auto& production_config = GetUniverse().economy_config.production_config;
    if (pl_ratio <= 0.1 || production_unit.continuous_gains <= production_config.construction_limit ||
        production_unit.utilization < production_unit.size || industry_node.all_of<components::Construction>()) {
        return;
    }
    // what's the ratio we should expand the factory at lolsize
    // Now we should expand it...
    // pl_ratio should be maybe
    // Set our construction costs
    if (recipenode.all_of<components::ConstructionCost>()) {
        const auto& construction_cost = recipenode.get<components::ConstructionCost>();
        // Let's assign construction costs
        auto& construction = industry_node.emplace<components::Construction>(
            0, construction_cost.time, static_cast<int>(production_unit.size * pl_ratio * 5));
    } else {
        auto& construction = industry_node.emplace<components::Construction>(
            0, 20, static_cast<int>(production_unit.size * pl_ratio * 5));
    }
}

/**
 * Returns true if we should continue with production false if we are constructing something
 */
bool SysProduction::HandleConstruction(Node& industry_node, components::Market& market) {
    ZoneScoped;
    if (!industry_node.any_of<components::Construction>()) {
        return true;
    }

    // Then progress construction
    auto& construction_progress = industry_node.get<components::Construction>();
    const auto& recipe_node = industry_node.Convert(industry_node.get<components::ProductionUnit>().recipe);
    // Now we should get our value...
    // Process construction costs
    // Add to market demand and cost
    if (recipe_node.all_of<components::ConstructionCost>()) {
        const auto& construction_cost = recipe_node.get<components::ConstructionCost>();
        market.consumption += construction_cost.cost;
        double price = construction_cost.cost.MultiplyAndGetSum(market.price);

        // then we should pass on the cost to who?
        // Next time we can add all our various financializations that we want
        // Let's just add it to the current wallet
        auto& wallet = industry_node.get<components::Wallet>();
        wallet -= price;
    }
    // If no shortage we progress construction
    construction_progress.progress++;
    int size = construction_progress.levels;
    if (construction_progress.progress >= construction_progress.maximum) {
        industry_node.get<components::ProductionUnit>().size += construction_progress.levels;
        industry_node.remove<components::Construction>();
    }
    // We don't have any construction?
    if (size == 0) {
        return false;
    }
    return false;
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
    auto& settlement = node.get<components::Settlement>();
    if (settlement.population.empty()) {
        return;
    }
    auto& market = node.get<components::Market>();
    market.GDP = 0;
    const auto& production_config = GetUniverse().economy_config.production_config;

    auto& industries = node.get<components::IndustrialZone>();
    Node population_node = node.Convert(settlement.population.front());

    // Get the transport cost
    auto& infrastructure = node.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;
    for (Node industry_node : node.Convert(industries.industries)) {
        // We should also check for industries we want to construct
        ProcessIndustry(industry_node, market, population_node, infra_cost);
    }
}

void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    // Each industrial zone is a a market
    int factories = 0;
    employed = 0;
    // Loop through the markets
    int settlement_count = 0;
    // Get the markets and process the values?
    IndustryFsm();
    for (Node entity : universe.nodes<components::IndustrialZone, components::Market>()) {
        ProcessIndustries(entity);
    }
    auto& population_history = universe.ctx().at<components::PopulationHistory>();
    population_history.employment.push_back(employed);
    population_history.employment_rate.push_back(employed / population_history.population.back() * 100.);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}

void SysProduction::IndustryFsm() {
    for (auto&& [industry, production] : GetUniverse().view<components::ProductionUnit>().each()) {
        ProductionPreprocessing(industry, production);
        components::IndustryState new_state;
        switch (production.state) {
            case components::IndustryState::SteadyState:
                new_state = SteadyState(industry, production);
                break;
            case components::IndustryState::MaximumProduction:
                new_state = MaximumProduction(industry, production);
                break;
            case components::IndustryState::MinimumProduction:
                new_state = MinimumProduction(industry, production);
                break;
            case components::IndustryState::Construction:
                new_state = Construction(industry, production);
                break;
            case components::IndustryState::Demolishing:
                new_state = Demolishing(industry, production);
                break;
            case components::IndustryState::Shrinking:
                new_state = Shrinking(industry, production);
                break;
            case components::IndustryState::Expanding:
                new_state = Expanding(industry, production);
                break;
            case components::IndustryState::Shortage:
                new_state = Shortage(industry, production);
                break;
        }
        if (new_state != production.state) {
            // Reset some values
            production.continuous_gains = 0;
            production.cumulative_pr = 0;
            production.stability = 0;
        }
        production.state = new_state;
    }
}

components::IndustryState SysProduction::SteadyState(entt::entity industry, components::ProductionUnit& production) {
    // Then if our gains are more we should do stuff
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Shrinking;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Expanding;
    }

    // Vary size by a few percent to simulate production variation
    production.diff = 1 + GetUniverse().random->GetRandomNormal(0, 0.0005);
    production.utilization *= production.diff;
    production.utilization = std::clamp(
        production.utilization * production.diff,
        GetUniverse().economy_config.production_config.factory_min_utilization * production.size, production.size);
    production.utilization = std::max(1., production.utilization);

    return components::IndustryState::SteadyState;
}
components::IndustryState SysProduction::MaximumProduction(entt::entity industry,
                                                           components::ProductionUnit& production) {
    if (production.continuous_gains > 30 * components::StarDate::DAY) {
        // We should set how much we want to construct
        // Actually we should let the FSM handle this
        if (GetUniverse().all_of<components::ConstructionCost>(production.recipe)) {
            const auto& construction_cost = GetUniverse().get<components::ConstructionCost>(production.recipe);
            // Let's assign construction costs
            auto& construction = GetUniverse().emplace<components::Construction>(
                industry, 0, construction_cost.time * components::StarDate::DAY,
                static_cast<int>(0.25 * production.size * production.ProfitMargin()));
        } else {
            auto& construction = GetUniverse().emplace<components::Construction>(
                industry, 0, 20 * components::StarDate::DAY,
                static_cast<int>(0.25 * production.size * production.ProfitMargin()));
        }
        return components::IndustryState::Construction;
    } else if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Shrinking;
    }
    return components::IndustryState::MaximumProduction;
}

components::IndustryState SysProduction::MinimumProduction(entt::entity industry,
                                                           components::ProductionUnit& production) {
    if (production.continuous_gains > 30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Demolishing;
    } else if (production.continuous_gains < -30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Expanding;
    }
    return components::IndustryState::MinimumProduction;
}

components::IndustryState SysProduction::Construction(entt::entity industry, components::ProductionUnit& production) {
    // If construction is done, we should continue
    auto& construction = GetUniverse().get<components::Construction>(industry);
    // Otherwise we should delete the cost
    auto recipe_node = GetUniverse()(production.recipe);
    if (recipe_node.all_of<components::ConstructionCost>()) {
        // Compute construction cost
        // TODO since we do have to talk to the market...
    }

    construction.progress += Interval();
    if (construction.progress >= construction.maximum) {
        production.size += construction.levels;
        GetUniverse().remove<components::Construction>(industry);
        return components::IndustryState::Expanding;
    }

    return components::IndustryState::Construction;
}

components::IndustryState SysProduction::Demolishing(entt::entity industry, components::ProductionUnit& production) {
    // Delete more stuff...
    return components::IndustryState::Demolishing;
}

components::IndustryState SysProduction::Shrinking(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Expanding;
    } else if (production.stability > 5 * components::StarDate::DAY) {
        // Then we should check for stability
        // We move to be steady much faster
        return components::IndustryState::SteadyState;
    }
    // Otherwise we should do something
    double diff = 1 +
                  economy_config.production_config.max_factory_delta /
                      (1 + std::exp(-(production.profit * economy_config.production_config.profit_multiplier))) -
                  economy_config.production_config.max_factory_delta / 2;

    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    production.diff = diff;
    production.utilization *= production.diff;
    production.utilization = std::clamp(
        production.utilization * production.diff,
        GetUniverse().economy_config.production_config.factory_min_utilization * production.size, production.size);
    production.utilization = std::max(1., production.utilization);
    return components::IndustryState::Shrinking;
}

components::IndustryState SysProduction::Expanding(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Shrinking;
    } else if (production.stability > 5 * components::StarDate::DAY) {
        // Then we should check for stability
        // We move to be steady much faster...
        if (production.utilization == production.size) {
            return components::IndustryState::MaximumProduction;
        }
        return components::IndustryState::SteadyState;
    }

    // If we are at the max production we should probably

    // Otherwise we should do something
    double diff = 1 +
                  economy_config.production_config.max_factory_delta /
                      (1 + std::exp(-(production.profit * economy_config.production_config.profit_multiplier))) -
                  economy_config.production_config.max_factory_delta / 2;

    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    production.diff = diff;
    production.utilization *= production.diff;
    production.utilization = std::clamp(
        production.utilization * production.diff,
        GetUniverse().economy_config.production_config.factory_min_utilization * production.size, production.size);
    production.utilization = std::max(1., production.utilization);
    return components::IndustryState::Expanding;  // No change
}

components::IndustryState SysProduction::Shortage(entt::entity industry, components::ProductionUnit& production) {
    // Cut production by like 10% lol
    // Also cut efficiency and stuff lol
    return components::IndustryState::Shortage;
}

void SysProduction::ProductionPreprocessing(entt::entity industry, components::ProductionUnit& production) {
    production.cumulative_pr += production.ProfitMargin();

    // How long we have continous gains and stuff?
    if (production.cumulative_pr > 5) {
        production.continuous_gains += Interval();
    } else if (production.cumulative_pr < -5) {
        production.continuous_gains -= Interval();
    } else {
        // How long we have a stable profit margin
        production.stability += Interval();
        production.continuous_gains = 0;
    }

    // Also check out underutilization and we can demolish things
    // Decomissioning stuff should be easier...
}
}  // namespace cqsp::core::systems
