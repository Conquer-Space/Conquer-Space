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
#include "core/components/needs.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
// Must be run after SysPopulationConsumption
// This is because population growth is dependent on if consumption was
// satisfied.
using components::PopulationConsumption;

void SysPopulationConsumption::ProcessSettlement(Node& settlement) {
    ZoneScoped;
    // Get the transport cost
    auto& settlement_comp = settlement.get<components::Settlement>();
    if (settlement_comp.population.empty()) {
        return;
    }
    if (!settlement.any_of<components::infrastructure::CityInfrastructure>()) {
        return;
    }

    auto& market = settlement.get<components::Market>();
    // Anything positive is a job that we want to shift away from, anything negative is something we want to shift to
    // Loop through the population segments through the settlements
    for (Node node_segment : settlement.Convert(settlement_comp.population)) {
        ProcessSegment(node_segment, market);
    }
}

void SysPopulationConsumption::ProcessSegment(Node& node_segment, components::Market& market) {
    ZoneScoped;
    // Compute things
    components::PopulationSegment& segment = node_segment.get<components::PopulationSegment>();
    PopulationConsumption& consumption = node_segment.get_or_emplace<PopulationConsumption>();
    // So our standard of living is a linear relationship with the other things
    // Compute what needs we have and do the math
    // Reduce pop to some unreasonably low level so that the economy can
    // handle it
    components::Wallet& wallet = node_segment.get_or_emplace<components::Wallet>();

    // So get our needs?
    // then we should compute our need contribution
    // So fill the needs and so on a and so forth
    // Also look for good subsititution
    // Fill our default goods or something..
    // Then something?
    double cost = 0;
    // So our needs will need to be computed

    // Our income should be equal to our spending...
    UpdateStandardOfLiving(segment);
    segment.average_wage = segment.income / (segment.employed_amount + 1);
    segment.spending = cost;
    // Add taxes to spending as well...
    // auto [consumption_cost, taxes] = market.PurchaseFromMarket(consumption);
    segment.income = segment.labor.labor_hours.MultiplyAndGetSum(market.price);
    // Also add income taxes based off a percentage
    // We assume people's income is uniform across stuff...
    // What about graduated income taxes lol
    wallet -= cost;  // Spend, even if it puts the pop into debt

    market.production += segment.labor.labor_hours;
    // market.consumption += consumption;
    total_sol += segment.standard_of_living * segment.population;
    total_population += segment.population;
    total_employed += segment.employed_amount;
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
        ProcessSettlement(settlement);
    }
    // Now compute our thing
    auto& history = GetUniverse().ctx().at<components::PopulationHistory>();
    history.population.push_back(total_population);
    history.sol.push_back(total_sol / static_cast<double>(total_population));
    history.employment.push_back(total_employed);
    history.employment_rate.push_back(static_cast<double>(total_employed) / static_cast<double>(total_population) *
                                      100.);
}

void SysPopulationConsumption::UpdateStandardOfLiving(components::PopulationSegment& segment) {
    double spending_ratio = (segment.income > 0) ? (segment.income - segment.spending) / segment.income : -1.0;

    segment.sol_pid.Update(std::clamp(spending_ratio, -2.0, 2.0));

    double sol_delta = 10 * segment.sol_pid.GetValue(components::PIDConfig {0.01, 0.1, 0.01});
    sol_delta = std::clamp(sol_delta, -segment.standard_of_living * 0.1, segment.standard_of_living * 0.1);
    segment.standard_of_living = std::max(segment.standard_of_living + sol_delta, 1.);
}

void SysPopulationConsumption::Init() {
    for (entt::entity cgentity : GetUniverse().consumergoods) {
        const components::ConsumerGood& good = GetUniverse().get<components::ConsumerGood>(cgentity);

        savings -= good.marginal_propensity;
    }  // These tables technically never need to be recalculated
    GetUniverse().ctx().emplace<components::PopulationHistory>();
}
}  // namespace cqsp::core::systems
