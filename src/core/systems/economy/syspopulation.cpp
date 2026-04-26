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

void SysPopulationGrowth::DoSystem() {
    ZoneScoped;

    for (Node pop_node : GetUniverse().nodes<components::PopulationSegment>()) {
        auto& segment = pop_node.get<components::PopulationSegment>();
        // If it's hungry, decay population
        if (pop_node.all_of<components::Hunger>()) {
            // Population decrease will be about 1 percent each year.
            float increase = 1.f - static_cast<float>(Interval()) * 0.00000114077116f;
            segment.population *= increase;
        }

        if (pop_node.all_of<components::FailedResourceTransfer>()) {
            // Then alert hunger.
            pop_node.get_or_emplace<components::Hunger>();
        } else {
            pop_node.remove<components::Hunger>();
        }
        // If not hungry, grow population
        if (!pop_node.all_of<components::Hunger>()) {
            // Population growth will be about 1 percent each year.
            float increase = static_cast<float>(Interval()) * 0.00000114077116f + 1;
            segment.population *= increase;
        }

        // Resolve jobs
        // TODO(EhWhoAmI)
        // For now, we would have 100% of the population working, because we
        // haven't got to social simulation yet. But in the future, this will
        // probably have to change.
        auto& employee = pop_node.get_or_emplace<components::LaborInformation>();
        employee.working_population = segment.population;
    }
}

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
    double tick_hours = (40. / static_cast<double>(components::StarDate::WEEK)) * Interval();
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

        // Compute labor hours
        segment.labor.labor_hours.clear();
        int workforce = 0;
        double hours_sum = 0;
        double employment_rate_sum = 0;
        int free_people = 0;
        std::map<entt::entity, int> target_jobs;
        for (auto& [labor, workers] : segment.labor.labor_distribution) {
            // Get the jobs that we are over and then figure out why
            auto& labor_comp = GetUniverse().get<components::Labor>(labor);
            // Shift our jobs a little towards jobs that need it and pay more...
            for (auto& demand : settlement_comp.job_demands) {
                if (demand.first == labor) {
                    // Then ignore or something ig
                    // Then otherwise distribute stuff
                    if (demand.second > 0) {
                        // Then this has jobs that are way over so we need to cut
                        int worker_diff = workers * 0.01;
                        workers -= worker_diff;
                        // Add to default jobs
                        free_people += worker_diff;
                    }
                    continue;
                } else {
                    // Otherwise we should check if the job is better or not...
                    // Check if we have more better paying jobs we can bounce to...
                    // Then we should account for the other thing
                    // Check if job is better paying
                    auto& current_labor = GetUniverse().get<components::Labor>(demand.first);
                    // Then we redistribute the jobs We should also add some inertia
                    if (market.price[current_labor.good] > market.price[labor_comp.good])
                        ;
                    {
                        // We add this to jobs
                        int worker_diff = workers * 0.01;
                        target_jobs[demand.first] += worker_diff;
                        // Then also decrease our workers...
                        workers -= worker_diff;
                    }
                }
            }

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

        for (auto& [labor, workers] : segment.labor.labor_distribution) {
            if (labor == GetUniverse().default_job) {
                workers += free_people;
            }
            // Check if we have to distribute people to jobs that are like not great or something
        }

        // Now redistribute our workers

        segment.employed_amount = employment_rate_sum;
        segment.unemployment_rate = (1 - employment_rate_sum) / workforce;

        // Our income should be equal to our spending...
        double spending_ratio = (segment.income > 0) ? (segment.income - segment.spending) / segment.income : -1.0;

        segment.sol_pid.Update(std::clamp(spending_ratio, -2.0, 2.0));

        double sol_delta = 200 * segment.sol_pid.GetValue(components::PIDConfig {0.01, 0.1, 0.01});
        sol_delta = std::clamp(sol_delta, -segment.standard_of_living * 0.1, segment.standard_of_living * 0.1);
        segment.standard_of_living = std::max(segment.standard_of_living + sol_delta, 1.);

        segment.average_wage = segment.income / (segment.employed_amount + 1);
        segment.spending = cost;
        segment.income = segment.labor.labor_hours.MultiplyAndGetSum(market.price);
        wallet -= cost;  // Spend, even if it puts the pop into debt

        market.production += segment.labor.labor_hours;
        market.consumption += consumption;
        total_sol += segment.standard_of_living * segment.population;
        total_population += segment.population;
        total_employed += segment.employed_amount;
    }
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

    auto labor_view = GetUniverse().view<components::LaborGood>();
    for (entt::entity entity : labor_view) {
        labor_goods.push_back(GetUniverse().good_map[entity]);
    }
}
}  // namespace cqsp::core::systems
