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

#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
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
    if (!settlement.any_of<components::infrastructure::CityInfrastructure>()) {
        return;
    }
    auto& infrastructure = settlement.get<components::infrastructure::CityInfrastructure>();
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;
    auto& market = settlement.get<components::Market>();
    // Loop through the population segments through the settlements
    auto& settlement_comp = settlement.get<components::Settlement>();
    for (Node node_segment : settlement.Convert(settlement_comp.population)) {
        // Compute things
        components::PopulationSegment& segment = node_segment.get_or_emplace<components::PopulationSegment>();
        ResourceConsumption& consumption = node_segment.get_or_emplace<ResourceConsumption>();
        // Reduce pop to some unreasonably low level so that the economy can
        // handle it
        const uint64_t population = segment.population / 10;

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

        double spending_ratio = (segment.income - segment.spending) / segment.income;
        if (spending_ratio > 0.1) {
            // Then we can increase SOL by 0.1
            segment.standard_of_living += 0.1 + segment.standard_of_living * 0.25;
        } else if (spending_ratio < 0.1) {
            segment.standard_of_living -= 0.1 + segment.standard_of_living * 0.25;
            segment.standard_of_living = std::max(segment.standard_of_living, 1.);
        }

        segment.spending = cost;
        segment.income = 0;
        segment.employed_amount = 0;
        wallet -= cost;  // Spend, even if it puts the pop into debt

        market.consumption += consumption;
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

    ResourceConsumption marginal_propensity_base;
    ResourceConsumption autonomous_consumption_base;
    float savings = 1;  // We calculate how much is saved since it is simpler
                        // than calculating spending
    for (entt::entity cgentity : universe.consumergoods) {
        const components::ConsumerGood& good = universe.get<components::ConsumerGood>(cgentity);
        marginal_propensity_base[universe.good_map[cgentity]] = good.marginal_propensity * Interval();
        autonomous_consumption_base[universe.good_map[cgentity]] = good.autonomous_consumption * Interval();
        savings -= good.marginal_propensity;
    }  // These tables technically never need to be recalculated

    for (Node settlement : universe.nodes<components::Settlement>()) {
        ProcessSettlement(settlement, marginal_propensity_base, autonomous_consumption_base, savings);
    }
}
}  // namespace cqsp::core::systems
