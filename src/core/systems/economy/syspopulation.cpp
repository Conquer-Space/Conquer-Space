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
#include "core/systems/economy/syspopulation.h"

#include <spdlog/spdlog.h>

#include <map>

#include <tracy/Tracy.hpp>

#include "core/components/history.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/population.h"
#include "core/components/resource.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
// Must be run after SysPopulationConsumption
// This is because population growth is dependent on if consumption was
// satisfied.

using components::ResourceConsumption;

void SysPopulationConsumption::ProcessSettlement(Node& settlement, const ResourceConsumption& marginal_propensity_base,
                                                 const ResourceConsumption& autonomous_consumption_base,
                                                 const float savings) {
    ZoneScoped;
    // Get the transport cost
    auto& settlement_comp = settlement.get<components::Settlement>();
    if (settlement_comp.population.empty()) {
        return;
    }
    if (!settlement.any_of<components::infrastructure::CityInfrastructure>()) {
        return;
    }
    // Also get our job stuff...
    auto& infrastructure = settlement.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;
    auto& market = settlement.get<components::Market>();
    // Anything positive is a job that we want to shift away from, anything negative is something we want to shift to
    // Loop through the population segments through the settlements
    for (Node node_segment : settlement.Convert(settlement_comp.population)) {
        // Compute things
        components::PopulationSegment& segment = node_segment.get_or_emplace<components::PopulationSegment>();
        ResourceConsumption& consumption = node_segment.get_or_emplace<ResourceConsumption>();
        // Reduce pop to some unreasonably low level so that the economy can
        // handle it
        const uint64_t population = segment.population;

        consumption = autonomous_consumption_base;

        // This value only changes when pop changes and
        // should be calculated in SysPopulationGrowth
        consumption *= population;

        components::Wallet& wallet = node_segment.get_or_emplace<components::Wallet>();
        double cost = (consumption * market.price).GetSum();

        if (wallet > 0) {  // If the pop has cash left over spend it
            // Add to the cost of price of transport
            const ResourceConsumption& extraconsumption = marginal_propensity_base;

            double extra_cost = (extraconsumption * market.price).GetSum();  // Distribute wallet amongst goods

            extra_cost *= segment.standard_of_living;

            // Now we should change the value that we do
            // Also see if we have extra money and then we can adjust SOL or something like that
            consumption += extraconsumption * segment.standard_of_living;  // Remove purchased goods from the market

            // Consumption
            // Check if there's enough on the market
            // Add the transport costs, and because they're importing it, we only account this
            cost += extra_cost;
        }

        HandleJob(segment, market);
        // Our income should be equal to our spending...
        double spending_ratio = (segment.income > 0) ? (segment.income - segment.spending) / segment.income : -1.0;

        segment.sol_pid.Update(std::clamp(spending_ratio, -2.0, 2.0));

        double sol_delta = 200 * segment.sol_pid.GetValue(components::PIDConfig {0.01, 0.1, 0.01});
        sol_delta = std::clamp(sol_delta, -segment.standard_of_living * 0.1, segment.standard_of_living * 0.1);
        segment.standard_of_living = std::max(segment.standard_of_living + sol_delta, 1.);

        segment.average_wage = segment.income / (segment.employed_amount + 1);
        segment.spending = cost;
        // Add taxes to spending as well...
        auto [consumption_cost, taxes] = market.PurchaseFromMarket(consumption);
        segment.income = segment.labor.labor_hours.MultiplyAndGetSum(market.price);
        // Also add income taxes based off a percentage
        // We assume people's income is uniform across stuff...
        // What about graduated income taxes lol
        wallet -= cost;  // Spend, even if it puts the pop into debt

        market.production += segment.labor.labor_hours;
        market.consumption += consumption;
        total_sol += segment.standard_of_living * segment.population;
        total_population += segment.population;
        total_employed += segment.employed_amount;
    }
}

void SysPopulationConsumption::HandleJob(components::PopulationSegment& segment, components::Market& market) {
    ZoneScoped;
    static const double tick_hours = (40. / static_cast<double>(components::StarDate::WEEK)) * Interval();
    segment.labor.labor_hours.clear();
    int workforce = 0;
    double hours_sum = 0;
    double employment_rate_sum = 0;

    std::map<entt::entity, int> job_drift;
    // Compute job drift
    for (auto& [labor, workers] : segment.labor.labor_distribution) {
        // Now sort through
        auto& labor_comp = GetUniverse().get<components::Labor>(labor);
        // Check how much job we should cut and stuff
        // So we need to loop through our market and see our possible jobs
        // In the future we should make a graph or something so that we don't need to check everything
        if (labor != GetUniverse().default_job && market.sd_ratio[labor_comp.good] > 2) {
            // Then we should probably start cutting
            // Find a ratio for amount we should cut...
            double delta = std::min((market.sd_ratio[labor_comp.good] - 2), 10.);
            double difference = workers * 0.01 * delta;
            job_drift[GetUniverse().default_job] += difference;
            job_drift[labor] -= difference;
        }
        // Also seek for more jobs
        for (const auto good : labor_goods) {
            if (market.price[good] - market.price[labor_comp.good] > market.price[labor_comp.good] * 0.1) {
                // It's enough money for the pop segment to want to shift their jobs
                // Like the thing is it has to be capped, it can't just swarm it...
                // Also cap by the amount of available good labors...
                double delta =
                    std::min((market.price[good] - market.price[labor_comp.good]) / market.price[labor_comp.good], 1.);
                double difference = workers * delta * 0.00001;
                double max_diff = std::max(segment.labor.labor_distribution[good_labors[good]] * 0.001, 100.);
                difference = std::min(max_diff, difference);
                job_drift[good_labors[good]] += difference;
                job_drift[labor] -= difference;
            }
        }
    }
    // Add the job drift
    for (auto& [labor, drift] : job_drift) {
        auto labor_count = std::max(1000, static_cast<int>(segment.labor.labor_distribution[labor] * 0.01));
        int drift_delta = std::clamp(drift, static_cast<int>(-labor_count), static_cast<int>(labor_count));
        segment.labor.labor_distribution[labor] += drift_delta;
        // Cap our labor distribution as well...
    }

    for (auto& [labor, workers] : segment.labor.labor_distribution) {
        // Get the jobs that we are over and then figure out why
        auto& labor_comp = GetUniverse().get<components::Labor>(labor);
        // Check if we are over
        // Shift our jobs a little towards jobs that need it and pay more...
        segment.labor.labor_hours.emplace_back(labor_comp.good, tick_hours * workers);
        workforce += workers;
        hours_sum += tick_hours * workers;
        double unemployment_rate = 1 / market.sd_ratio[labor_comp.good];

        // Then we should do something about it
        // If we are way over we are also overemployed...
        // Check if we are way over and if we are way over we should dump jobs
        // Also check job drift to higher paying jobs
        employment_rate_sum += workers * unemployment_rate;
    }

    // Now redistribute our workers

    segment.employed_amount = employment_rate_sum;
    segment.unemployment_rate = (1 - employment_rate_sum) / workforce;
}

// In economics, the consumption function describes a relationship between
// consumption and disposable income.
// Its simplest form is the linear consumption function used frequently in
// simple Keynesian models For each consumer good consumption is modeled as
// follows C = a + b * Y C represents consumption of the consumer good a
// represents autonomous consumption that is independent of disposable income or
// when income levels are zero
//  if income levels cannot pay for this level of maintenance they are drawn from
//  the population's savings or debt
// b represents the marginal propensity (demand) to consume or how
//  much of their surplus income they will spend on that consumer good
//  Based on how many consumer goods they consume from this segment, we can find
//  their economic strata.
// Y represents their disposable income (funds left over from last tick)
//  Population savings can be ensured by keeping the sum of all b values below 1
// Consumer Goods Include
//  - Housing
//  - Transport
//  - Healthcare
//  - Insurance
//  - Entertainment
//  - Education
//  - Utilities
// Cash from money printing is directly injected into the population
// This represents basic subsdies and providing an injection point for new cash.
// Eventually this should be moved to two seperate and unique systems
void SysPopulationConsumption::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    total_population = 0;
    total_sol = 0;
    total_employed = 0;
    for (Node settlement : universe.nodes<components::Settlement>()) {
        ProcessSettlement(settlement, marginal_propensity_base, autonomous_consumption_base, savings);
    }
    // Now compute our thing
    auto& history = GetUniverse().ctx().at<components::PopulationHistory>();
    history.population.push_back(total_population);
    history.sol.push_back(total_sol / static_cast<double>(total_population));
    history.employment.push_back(total_employed);
    history.employment_rate.push_back(static_cast<double>(total_employed) / static_cast<double>(total_population) *
                                      100.);
}

void SysPopulationConsumption::Init() {
    for (entt::entity cgentity : GetUniverse().consumergoods) {
        const components::ConsumerGood& good = GetUniverse().get<components::ConsumerGood>(cgentity);
        marginal_propensity_base[GetUniverse().good_map[cgentity]] = good.marginal_propensity * Interval();
        autonomous_consumption_base[GetUniverse().good_map[cgentity]] = good.autonomous_consumption * Interval();
        savings -= good.marginal_propensity;
    }  // These tables technically never need to be recalculated
    GetUniverse().ctx().emplace<components::PopulationHistory>();

    auto labor_view = GetUniverse().view<components::Labor>();
    for (auto&& [entity, comp] : labor_view.each()) {
        labor_goods.push_back(comp.good);
        good_labors[comp.good] = entity;
    }
}
}  // namespace cqsp::core::systems
