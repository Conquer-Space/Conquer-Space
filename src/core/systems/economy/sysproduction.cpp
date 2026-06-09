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
double SysProduction::ProcessIndustry(Node& industry_node, Node& market_node, components::Market& market,
                                      double infra_cost) {
    ZoneScoped;
    double tax_income = 0;
    auto& employer = industry_node.get<components::Employer>();
    auto& wallet = industry_node.get<components::Wallet>();

    // Process imdustries
    // Industries MUST have production and a linked recipe
    if (!industry_node.all_of<components::ProductionUnit>()) return 0.0;

    components::ProductionUnit& size = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(size.recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();

    size.construction_cost = 0;
    size.tax_cost = 0;
    if (size.state == components::IndustryState::Construction) {
        auto& construction_sector = market_node.get<components::infrastructure::ConstructionSector>();
        // then we also do the construuction input
        // We should probably also do the shortage as well
        auto& construction = industry_node.get<components::Construction>();
        // Now progress by one and stuff
        // Then get the recipe cost as well
        auto& construction_cost = recipenode.get<components::ConstructionCost>();
        // How do we process construction
        // Get the amount that we cost and add to the cost
        // construction.levels * construction_sector.construction_cost
        double construction_amount = construction.levels * Interval();
        components::ResourceVector cost_vector = construction_cost.cost * construction.levels;
        bool construction_shortage = false;
        for (auto& [good, amount] : cost_vector) {
            if (market.chronic_shortages[good] > 5) {
                // Reduce the amount based off the weighted average of the input?
                // Then reduce production over time or something
                construction_shortage = true;
                break;
            }
        }
        if (construction_shortage) {
            // Then we should reduce construction rate
            construction_amount = construction_amount * 0.1;
        }
        construction.progress += construction_amount;
        construction_sector.current_construction += construction_amount;
        auto [cost, tax] = market.PurchaseFromMarket(cost_vector);
        wallet -= cost + tax;
        tax_income += tax;
        size.construction_cost = cost;
        size.tax_cost += tax;
    }
    // Let's calculate the size from previous input
    // Calculate resource consumption
    components::ResourceVector capitalinput = recipe.capitalcost * (size.size);
    components::ResourceVector input = (recipe.input * size.utilization) + capitalinput;

    // Calculate the greatest possible production
    components::ResourceVector output;
    output.push_back(std::pair(recipe.output.entity, recipe.output.amount * size.utilization));

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

    // Configure how many hours we should hire...
    // We should also drift wages towards the average wage of the pop node or something
    // Just set our factory hiring?
    // TODO(EhWhoAmI): This is definitely pretty bad for performance so we will have to fix this or tweak it
    // Chances are we just add a good map for labor in the recipe map
    size.workers.clear();
    for (const auto& [job, workers] : recipe.workers.workers) {
        auto& labor = GetUniverse().get<components::Labor>(job);
        size.workers.emplace_back(labor.good, workers * size.utilization);
        if (market.chronic_shortages[labor.good] > 5) {
            SPDLOG_INFO("Shortage in labor!");
            shortage = true;
        }
    }
    size.shortage = shortage;
    market.production += output;
    market.consumption += size.workers;
    size.amount_sold = recipe.output.amount * size.utilization;

    double output_transport_cost = output.GetSum() * infra_cost;
    double input_transport_cost = input.GetSum() * infra_cost;
    // Next time need to compute the costs along with input and
    // output so that the factory doesn't overspend. We sorta
    // need a balanced economy

    // Maintenance costs will still have to be upkept, so if
    // there isnt any resources to upkeep the place, then stop
    // the production
    auto [material_costs, taxes] = market.PurchaseFromMarket(input);
    size.material_costs = material_costs;

    auto [wage_costs, income_taxes] = market.PurchaseFromMarket(input);

    size.tax_cost += taxes + income_taxes;
    size.wage_cost = wage_costs;
    size.transport = 0;  //output_transport_cost + input_transport_cost;

    size.revenue = output.MultiplyAndGetSum(market.price);
    size.profit =
        size.revenue - size.maintenance - size.material_costs - size.wage_cost - size.transport - size.tax_cost;
    wallet += size.profit;
    market.GDP += size.revenue - size.material_costs;
    tax_income += taxes + income_taxes;
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
    return tax_income;
}

void SysProduction::ScaleConstruction(Node& industry_node, double pl_ratio) {
    ZoneScoped;
    components::ProductionUnit& production_unit = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(production_unit.recipe);
    components::Recipe recipe = recipenode.get<components::Recipe>();
    if (pl_ratio <= 0.1 || production_unit.continuous_gains <= production_config.construction_limit ||
        production_unit.utilization < production_unit.size || industry_node.all_of<components::Construction>()) {
        return;
    }
    // what's the ratio we should expand the factory at lolsize
    // Now we should expand it...
    // pl_ratio should be maybe
    // Set our construction costs
    int to_construct = static_cast<int>(production_unit.size * pl_ratio * 5);
    if (recipenode.all_of<components::ConstructionCost>()) {
        const auto& construction_cost = recipenode.get<components::ConstructionCost>();
        // Let's assign construction costs
        // then we can assign the amount of stuff to this?
        auto& construction =
            industry_node.emplace<components::Construction>(0, construction_cost.time * to_construct, to_construct);
    } else {
        auto& construction = industry_node.emplace<components::Construction>(0, 20 * to_construct, to_construct);
    }
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

    auto& industries = node.get<components::IndustrialZone>();

    // Get the transport cost
    auto& infrastructure = node.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;
    double total_taxes = 0;
    for (Node industry_node : node.Convert(industries.industries)) {
        // We should also check for industries we want to construct
        total_taxes += ProcessIndustry(industry_node, node, market, infra_cost);
    }
    // Now get province and stuff
    auto& province = node.get<components::Province>();
    auto& income = GetUniverse().get<components::OrganizationIncome>(province.country);
    income.income_taxes += total_taxes;
}

void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    // Each industrial zone is a a market
    int factories = 0;
    // Loop through the markets
    int settlement_count = 0;
    // Get the markets and process the values?
    // Reset construction
    for (auto&& [entity, construction, market] :
         universe.view<components::infrastructure::ConstructionSector, components::Market>().each()) {
        construction.current_construction = 0;
    }

    IndustryFsm();
    for (Node entity : universe.nodes<components::IndustrialZone, components::Market>()) {
        ProcessIndustries(entity);
    }
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
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.factory_min_utilization * production.size, production.size);
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
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Demolishing;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        return components::IndustryState::Expanding;
    }
    return components::IndustryState::MinimumProduction;
}

components::IndustryState SysProduction::Construction(entt::entity industry, components::ProductionUnit& production) {
    auto& construction = GetUniverse().get<components::Construction>(industry);

    if (construction.progress >= construction.maximum) {
        production.size += construction.levels;
        GetUniverse().remove<components::Construction>(industry);
        return components::IndustryState::Expanding;
    }

    return components::IndustryState::Construction;
}

components::IndustryState SysProduction::Demolishing(entt::entity industry, components::ProductionUnit& production) {
    double delta = 0.25 * production.size * production.ProfitMargin();
    delta = std::clamp(delta, -production.size * 0.25, 0.);
    production.size += delta;
    production.size = std::max(production.size, 1.);
    return components::IndustryState::Shrinking;
}

components::IndustryState SysProduction::Shrinking(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains > 30 * components::StarDate::DAY) {
        return components::IndustryState::Expanding;
    } else if (production.stability > 5 * components::StarDate::DAY) {
        // Then we should check for stability
        // We move to be steady much faster
        double min_utilization = production_config.GetFactoryMinUtilization(production.size);
        if (production.utilization == min_utilization) {
            return components::IndustryState::MinimumProduction;
        }
        return components::IndustryState::SteadyState;
    } else if (production.continuous_gains < -30 * components::StarDate::DAY) {
        double min_utilization = production_config.GetFactoryMinUtilization(production.size);
        if (production.utilization == min_utilization) {
            return components::IndustryState::MinimumProduction;
        }
    }
    // Otherwise we should do something
    double diff = production_config.GetFactoryUtilizationDiff(production.profit);

    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    // Make sure we don't increase in size
    diff = std::min(diff, 1.);

    production.diff = diff;
    production.utilization *= production.diff;
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.GetFactoryMinUtilization(production.size), production.size);
    return components::IndustryState::Shrinking;
}

components::IndustryState SysProduction::Expanding(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Shrinking;
    } else if (production.stability > 5 * components::StarDate::DAY) {
        // Then we should check for stability
        // We move to be steady much faster...
        if (production.utilization >= production.size - 1) {
            return components::IndustryState::MaximumProduction;
        }
        return components::IndustryState::SteadyState;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        if (production.utilization >= production.size - 1) {
            return components::IndustryState::MaximumProduction;
        }
    }
    // Otherwise we should do something
    double diff = production_config.GetFactoryUtilizationDiff(production.profit);

    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    // Ensure we actually expand
    diff = std::max(diff, 1.);
    production.diff = diff;
    production.utilization *= production.diff;
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.GetFactoryMinUtilization(production.size), production.size);
    return components::IndustryState::Expanding;  // No change
}

components::IndustryState SysProduction::Shortage(entt::entity industry, components::ProductionUnit& production) {
    // This cuts production by more
    if (!production.shortage) {
        return components::IndustryState::SteadyState;
    }
    production.diff = std::max(GetUniverse().random->GetRandomNormal(0.1, 0.1), 0.02);

    // Shortages have no limit for how little we can go
    production.utilization = std::clamp(production.utilization * production.diff, 1., production.size);
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

    // Force shortage
    if (production.shortage) {
        production.state = components::IndustryState::Shortage;
    }
}
}  // namespace cqsp::core::systems
