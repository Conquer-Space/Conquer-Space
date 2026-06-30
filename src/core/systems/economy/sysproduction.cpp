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
// Entry point: resets construction counters, advances the industry FSM, then runs production for every settlement.
void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    for (auto&& [entity, construction, market] :
         universe.view<components::infrastructure::ConstructionSector, components::Market>().each()) {
        construction.current_construction = 0;
    }

    IndustryFsm();
    for (Node entity : universe.nodes<components::IndustrialZone, components::Market>()) {
        ProcessIndustries(entity);
    }
}

// Advances each industry's state machine by one tick. State transitions drive capacity expansion/contraction.
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
            production.continuous_gains = 0;
            production.cumulative_pr = 0;
            production.stability = 0;
        }
        production.state = new_state;
    }
}

// Accumulates profit signal into continuous_gains/stability counters that the FSM reads to decide state transitions.
void SysProduction::ProductionPreprocessing(entt::entity industry, components::ProductionUnit& production) {
    production.cumulative_pr += production.ProfitMargin();

    if (production.cumulative_pr > 5) {
        production.continuous_gains += Interval();
    } else if (production.cumulative_pr < -5) {
        production.continuous_gains -= Interval();
    } else {
        production.stability += Interval();
        production.continuous_gains = 0;
    }

    if (production.shortage) {
        production.state = components::IndustryState::Shortage;
    }
}

// Steady operation: adds small random noise to utilization; transitions to Shrinking or Expanding on sustained losses/gains.
components::IndustryState SysProduction::SteadyState(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Shrinking;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        production.continuous_gains = 0;
        return components::IndustryState::Expanding;
    }

    production.diff = 1 + GetUniverse().random->GetRandomNormal(0, 0.0005);
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.factory_min_utilization * production.size, production.size);
    production.utilization = std::max(1., production.utilization);

    return components::IndustryState::SteadyState;
}

// At full capacity: triggers a Construction job if gains persist, or falls back to Shrinking on losses.
components::IndustryState SysProduction::MaximumProduction(entt::entity industry,
                                                           components::ProductionUnit& production) {
    if (production.continuous_gains > 30 * components::StarDate::DAY) {
        if (GetUniverse().all_of<components::ConstructionCost>(production.recipe)) {
            const auto& construction_cost = GetUniverse().get<components::ConstructionCost>(production.recipe);
            GetUniverse().emplace<components::Construction>(
                industry, 0, construction_cost.time * components::StarDate::DAY,
                static_cast<int>(0.25 * production.size * production.ProfitMargin()));
        } else {
            GetUniverse().emplace<components::Construction>(
                industry, 0, 20 * components::StarDate::DAY,
                static_cast<int>(0.25 * production.size * production.ProfitMargin()));
        }
        return components::IndustryState::Construction;
    } else if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Shrinking;
    }
    return components::IndustryState::MaximumProduction;
}

// At minimum viable utilization: demolishes on sustained losses, expands back out on gains.
components::IndustryState SysProduction::MinimumProduction(entt::entity industry,
                                                           components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Demolishing;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        return components::IndustryState::Expanding;
    }
    return components::IndustryState::MinimumProduction;
}

// Waits for the Construction component's progress to reach its maximum, then adds the new capacity and transitions to Expanding.
components::IndustryState SysProduction::Construction(entt::entity industry, components::ProductionUnit& production) {
    auto& construction = GetUniverse().get<components::Construction>(industry);
    if (construction.progress >= construction.maximum) {
        production.size += construction.levels;
        GetUniverse().remove<components::Construction>(industry);
        return components::IndustryState::Expanding;
    }
    return components::IndustryState::Construction;
}

// Shrinks physical capacity proportional to the profit margin, capped at 25% per tick, then moves to Shrinking.
components::IndustryState SysProduction::Demolishing(entt::entity industry, components::ProductionUnit& production) {
    double delta = 0.25 * production.size * production.ProfitMargin();
    delta = std::clamp(delta, -production.size * 0.25, 0.);
    production.size += delta;
    production.size = std::max(production.size, 1.);
    return components::IndustryState::Shrinking;
}

// Ramps utilization down; stabilizes to SteadyState/MinimumProduction or reverses to Expanding if profit recovers.
components::IndustryState SysProduction::Shrinking(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains > 30 * components::StarDate::DAY) {
        return components::IndustryState::Expanding;
    } else if (production.stability > 5 * components::StarDate::DAY) {
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

    double diff = production_config.GetFactoryUtilizationDiff(production.profit);
    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    diff = std::min(diff, 1.);

    production.diff = diff;
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.GetFactoryMinUtilization(production.size), production.size);
    return components::IndustryState::Shrinking;
}

// Ramps utilization up toward full capacity; transitions to MaximumProduction when full or SteadyState on stabilization.
components::IndustryState SysProduction::Expanding(entt::entity industry, components::ProductionUnit& production) {
    if (production.continuous_gains < -30 * components::StarDate::DAY) {
        return components::IndustryState::Shrinking;
    } else if (production.stability > 5 * components::StarDate::DAY) {
        if (production.utilization >= production.size - 1) {
            return components::IndustryState::MaximumProduction;
        }
        return components::IndustryState::SteadyState;
    } else if (production.continuous_gains > 30 * components::StarDate::DAY) {
        if (production.utilization >= production.size - 1) {
            return components::IndustryState::MaximumProduction;
        }
    }

    double diff = production_config.GetFactoryUtilizationDiff(production.profit);
    diff += GetUniverse().random->GetRandomNormal(0, 0.0005);
    diff = std::max(diff, 1.);
    production.diff = diff;
    production.utilization = std::clamp(production.utilization * production.diff,
                                        production_config.GetFactoryMinUtilization(production.size), production.size);
    return components::IndustryState::Expanding;
}

// Aggressively cuts utilization when inputs are unavailable; exits to SteadyState once the shortage clears.
components::IndustryState SysProduction::Shortage(entt::entity industry, components::ProductionUnit& production) {
    if (!production.shortage) {
        return components::IndustryState::SteadyState;
    }
    production.diff = std::max(GetUniverse().random->GetRandomNormal(0.1, 0.1), 0.02);
    production.utilization = std::clamp(production.utilization * production.diff, 1., production.size);
    return components::IndustryState::Shortage;
}

// Maps industry state to per-tick expertise delta. Shrinking/demolishing states lose expertise; shortage gains almost nothing.
double SysProduction::StateToExpertiseGain(components::IndustryState state) {
    switch (state) {
        case components::IndustryState::SteadyState:
            return 0.0001;
        case components::IndustryState::MaximumProduction:
            return 0.0001;
        case components::IndustryState::MinimumProduction:
            return 0.0001;
        case components::IndustryState::Construction:
            return 0.000;
        case components::IndustryState::Demolishing:
            return -0.0001;
        case components::IndustryState::Shrinking:
            return -0.0001;
        case components::IndustryState::Expanding:
            return -0.0001;
        case components::IndustryState::Shortage:
            return 0.00001;
    }
}

// Iterates all industries in a settlement, accumulates their tax contributions, and credits them to the province's country.
void SysProduction::ProcessIndustries(Node& node) {
    ZoneScoped;
    auto& settlement = node.get<components::Settlement>();
    if (settlement.population.empty()) {
        return;
    }
    auto& market = node.get<components::Market>();
    market.GDP = 0;

    auto& industries = node.get<components::IndustrialZone>();
    auto& infrastructure = node.get<components::infrastructure::CityInfrastructure>();
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    double total_taxes = 0;
    for (Node industry_node : node.Convert(industries.industries)) {
        total_taxes += ProcessIndustry(industry_node, node, market, infra_cost);
    }

    auto& province = node.get<components::Province>();
    auto& income = GetUniverse().get<components::OrganizationIncome>(province.country);
    income.income_taxes += total_taxes;
}

// Runs one production tick for a single industry: handles construction spending, consumes inputs, produces outputs,
// computes revenue/profit/subsidies, and returns the tax income generated.
double SysProduction::ProcessIndustry(Node& industry_node, Node& market_node, components::Market& market,
                                      double infra_cost) {
    ZoneScoped;
    if (!industry_node.all_of<components::ProductionUnit>()) return 0.0;

    auto& employer = industry_node.get<components::Employer>();
    auto& wallet = industry_node.get<components::Wallet>();
    components::ProductionUnit& size = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(size.recipe);
    const components::Recipe& recipe = recipenode.get<components::Recipe>();

    // Construction spending: only runs while the industry is actively being built.
    size.construction_cost = 0;
    size.tax_cost = 0;
    double tax_income = 0;
    if (size.state == components::IndustryState::Construction) {
        tax_income += ProcessConstruction(industry_node, market_node, market);
    }

    // Input vectors: capital upkeep scales with physical size, operational inputs scale with utilization.
    components::ResourceVector capitalinput = recipe.capitalcost * (size.size);
    components::ResourceVector input = (recipe.input * size.utilization) + capitalinput;

    // Expertise tick: working industries gain mastery over time, boosting output per unit of utilization.
    size.expertise_gain = StateToExpertiseGain(size.state);
    size.expertise += size.expertise_gain;
    size.expertise = std::clamp(static_cast<double>(size.expertise), 0., static_cast<double>(size.max_expertise));

    // Output vector: single good, scaled by utilization and expertise multiplier.
    components::ResourceVector output;
    size.amount_sold = recipe.output.amount * size.utilization * size.expertise;
    output.push_back(std::pair(recipe.output.entity, size.amount_sold));

    // Shortage detection: flag if any material input or required labor type is chronically undersupplied.
    bool shortage = false;
    for (auto& [good, amount] : recipe.input) {
        if (market.chronic_shortages[good] > 5) {
            shortage = true;
            break;
        }
    }

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

    // Register this industry's production and labor demand with the market for this tick.
    market.production += output;

    // Purchase inputs and labor from the market; taxes are collected on both transactions.
    auto [material_costs, taxes] = market.PurchaseFromMarket(input);
    size.material_costs = material_costs;

    auto [wage_costs, income_taxes] = market.PurchaseFromMarket(size.workers);
    size.tax_cost += taxes + income_taxes;
    size.wage_cost = wage_costs;
    size.transport = 0;

    // Profit = revenue minus all costs. A negative profit signals the FSM to eventually shrink.
    size.revenue = output.MultiplyAndGetSum(market.price);
    size.profit =
        size.revenue - size.maintenance - size.material_costs - size.wage_cost - size.transport - size.tax_cost;

    // Subsidy application: < 1.0 is a partial profit subsidy; >= 1.0 fully covers any loss.
    if (size.output_subsidy < 1.0) {
        size.output_subsidy_amount = size.profit * size.output_subsidy;
        size.profit += size.output_subsidy_amount;
    } else {
        if (size.profit < 0.0) {
            size.output_subsidy_amount = -size.profit;
            size.profit = 0;
        } else {
            size.output_subsidy_amount = 0;
        }
    }

    // Commit profit to the industry's wallet and value-added to market GDP.
    wallet += size.profit;
    market.GDP += size.revenue - size.material_costs;
    tax_income += taxes + income_taxes - size.output_subsidy_amount;
    return tax_income;
}

// Advances construction progress for one tick: purchases materials from the market, pays construction sector fees,
// and halves the build rate if any required good is in chronic shortage.
double SysProduction::ProcessConstruction(Node& industry_node, Node& market_node, components::Market& market) {
    ZoneScoped;
    auto& size = industry_node.get<components::ProductionUnit>();
    auto& wallet = industry_node.get<components::Wallet>();
    Node recipenode = industry_node.Convert(size.recipe);

    auto& construction_sector = market_node.get<components::infrastructure::ConstructionSector>();
    auto& construction = industry_node.get<components::Construction>();
    auto& construction_cost = recipenode.get<components::ConstructionCost>();

    double construction_amount = construction.levels * Interval();
    components::ResourceVector cost_vector = construction_cost.cost * construction.levels;

    bool shortage = false;
    for (auto& [good, amount] : cost_vector) {
        if (market.chronic_shortages[good] > 5) {
            shortage = true;
            break;
        }
    }
    if (shortage) {
        construction_amount *= 0.1;
    }

    construction.progress += construction_amount;
    construction_sector.current_construction += construction_amount;
    auto [cost, tax] = market.PurchaseFromMarket(cost_vector);
    double sector_cost = construction_sector.construction_cost * construction_amount;
    wallet -= cost + tax + sector_cost;
    size.construction_cost = cost + sector_cost;
    size.tax_cost += tax;
    return tax;
}

// Initiates a construction job sized proportional to pl_ratio, skipped if already building or underutilized.
void SysProduction::ScaleConstruction(Node& industry_node, double pl_ratio) {
    ZoneScoped;
    components::ProductionUnit& production_unit = industry_node.get<components::ProductionUnit>();
    Node recipenode = industry_node.Convert(production_unit.recipe);
    if (pl_ratio <= 0.1 || production_unit.continuous_gains <= production_config.construction_limit ||
        production_unit.utilization < production_unit.size || industry_node.all_of<components::Construction>()) {
        return;
    }
    int to_construct = static_cast<int>(production_unit.size * pl_ratio * 5);
    if (recipenode.all_of<components::ConstructionCost>()) {
        const auto& construction_cost = recipenode.get<components::ConstructionCost>();
        industry_node.emplace<components::Construction>(0, construction_cost.time * to_construct, to_construct);
    } else {
        industry_node.emplace<components::Construction>(0, 20 * to_construct, to_construct);
    }
}
}  // namespace cqsp::core::systems
