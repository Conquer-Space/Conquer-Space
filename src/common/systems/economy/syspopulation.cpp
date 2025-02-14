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
#include "common/systems/economy/syspopulation.h"

#include <spdlog/spdlog.h>

#include <tracy/Tracy.hpp>

#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/surface.h"

namespace cqspc = cqsp::common::components;

namespace cqsp::common::systems {
// Must be run after SysPopulationConsumption
// This is because population growth is dependent on if consumption was
// satisfied.
void SysPopulationGrowth::DoSystem() {
    ZoneScoped;

    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (entt::entity entity : view) {
        auto& segment = universe.get<cqspc::PopulationSegment>(entity);
        // If it's hungry, decay population
        if (universe.all_of<cqspc::Hunger>(entity)) {
            // Population decrease will be about 1 percent each year.
            float increase = 1.f - static_cast<float>(Interval()) * 0.00000114077116f;
            segment.population *= increase;
        }

        if (universe.all_of<cqspc::FailedResourceTransfer>(entity)) {
            // Then alert hunger.
            universe.get_or_emplace<cqspc::Hunger>(entity);
        } else {
            universe.remove<cqspc::Hunger>(entity);
        }
        // If not hungry, grow population
        if (!universe.all_of<cqspc::Hunger>(entity)) {
            // Population growth will be about 1 percent each year.
            float increase = static_cast<float>(Interval()) * 0.00000114077116f + 1;
            segment.population *= increase;
        }

        // Resolve jobs
        // TODO(EhWhoAmI)
        // For now, we would have 100% of the population working, because we
        // haven't got to social simulation yet. But in the future, this will
        // probably have to change.
        auto& employee = universe.get_or_emplace<cqspc::LaborInformation>(entity);
        employee.working_population = segment.population;
    }
}

namespace {
void ProcessSettlement(cqsp::common::Universe& universe, entt::entity settlement, cqspc::Market& market,
                       cqspc::ResourceConsumption& marginal_propensity_base,
                       cqspc::ResourceConsumption& autonomous_consumption_base, float savings) {
    // Get the transport cost
    auto& infrastructure = universe.get<cqspc::infrastructure::CityInfrastructure>(settlement);
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    // Loop through the population segments through the settlements
    auto& settlement_comp = universe.get<cqspc::Settlement>(settlement);
    for (entt::entity segmententity : settlement_comp.population) {
        // Compute things
        cqspc::PopulationSegment& segment = universe.get_or_emplace<cqspc::PopulationSegment>(segmententity);
        cqspc::ResourceConsumption& consumption = universe.get_or_emplace<cqspc::ResourceConsumption>(segmententity);
        // Reduce pop to some unreasonably low level so that the economy can
        // handle it
        const uint64_t population = segment.population / 10;

        consumption = autonomous_consumption_base;

        // This value only changes when pop changes and
        // should be calculated in SysPopulationGrowth
        consumption *= population;

        cqspc::Wallet& wallet = universe.get_or_emplace<cqspc::Wallet>(segmententity);
        const double cost = (consumption * market.price).GetSum();
        wallet -= cost;    // Spend, even if it puts the pop into debt
        if (wallet > 0) {  // If the pop has cash left over spend it
            // Add to the cost of price of transport
            cqspc::ResourceConsumption extraconsumption = marginal_propensity_base;
            // Loop through all the things, if there isn't enough resources for a
            // If the market supply has all of the goods, then they can buy the goods
            // Get previous market supply
            // the total consumption
            // Add to the cost
            // They can buy less because of things
            //extraconsumption *= infra_cost;
            extraconsumption *= wallet;        // Distribute wallet amongst goods
            extraconsumption /= market.price;  // Find out how much of each good you can buy
            consumption += extraconsumption;   // Remove purchased goods from the market
            for (auto& t : consumption) {
                // Look for in the market, and then if supply is zero, then deny them buying
                if (market.previous_supply[t.first] <= 0) {
                    // Then they cannot buy the stuff
                    // Then do the consumption
                    // Add to latent demand
                    market.latent_demand[t.first] += t.second;
                    t.second = 0;
                }
            }
            // Consumption
            // Check if there's enough on the market
            // Add the transport costs, and because they're importing it, we only account this
            double cost = consumption.GetSum() * infra_cost;
            wallet *= savings;  // Update savings
            wallet -= cost;
        }

        // TODO(EhWhoAmI): Don't inject cash, take the money from the government
        wallet += segment.population * 50000;  // Inject cash

        market.demand += consumption;
    }
}
}  // namespace

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

    cqspc::ResourceConsumption marginal_propensity_base;
    cqspc::ResourceConsumption autonomous_consumption_base;
    float savings = 1;  // We calculate how much is saved since it is simpler
                        // than calculating spending
    for (entt::entity cgentity : universe.consumergoods) {
        const cqspc::ConsumerGood& good = universe.get<cqspc::ConsumerGood>(cgentity);
        marginal_propensity_base[cgentity] = good.marginal_propensity;
        autonomous_consumption_base[cgentity] = good.autonomous_consumption;
        savings -= good.marginal_propensity;
    }  // These tables technically never need to be recalculated
    auto settlementview = universe.view<cqspc::Settlement>();

    // Loop through the settlements on a planet, then process the market?
    auto market_view = universe.view<cqspc::Habitation>();
    int settlement_count = 0;
    for (entt::entity entity : market_view) {
        // Get the children, because reasons
        // All planets with a habitation WILL have a market
        auto& market = universe.get_or_emplace<cqspc::Market>(entity);
        // Read the segment information
        auto& habit = universe.get<cqspc::Habitation>(entity);
        for (entt::entity settlement : habit.settlements) {
            ProcessSettlement(universe, settlement, market, marginal_propensity_base, autonomous_consumption_base,
                              savings);
            settlement_count++;
        }
    }
    SPDLOG_TRACE("Processing {} settlements in {} markets", settlement_count, market_view.size());
}
}  // namespace cqsp::common::systems
