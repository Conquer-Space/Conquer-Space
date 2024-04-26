/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

namespace components = cqsp::common::components;
namespace infrastructure = components::infrastructure;

using cqsp::common::systems::SysPopulationGrowth;
using cqsp::common::systems::SysPopulationConsumption;
using components::PopulationSegment;
using components::Settlement;
using components::Wallet;
using components::Hunger;
using components::FailedResourceTransfer;
using components::LaborInformation;
using components::ResourceConsumption;
using components::ConsumerGood;
using components::Habitation;
using components::Market;
using entt::entity;

// Must be run after SysPopulationConsumption
// This is because population growth is dependent on if consumption was
// satisfied.
void SysPopulationGrowth::DoSystem() {
    ZoneScoped;

    Universe& universe = GetUniverse();
    for (entity entity : universe.view<PopulationSegment>()) {
        auto& segment = universe.get<PopulationSegment>(entity);
        // If it's hungry, decay population
        if (universe.all_of<Hunger>(entity)) {
            // Population decrease will be about 1 percent each year.
            float increase = 1.f - static_cast<float>(Interval()) * 0.00000114077116f;
            segment.population *= increase;
        }

        if (universe.all_of<FailedResourceTransfer>(entity)) {
            // Then alert hunger.
            universe.get_or_emplace<Hunger>(entity);
        } else {
            universe.remove<Hunger>(entity);
        }
        // If not hungry, grow population
        if (!universe.all_of<Hunger>(entity)) {
            // Population growth will be about 1 percent each year.
            float increase = static_cast<float>(Interval()) * 0.00000114077116f + 1;
            segment.population *= increase;
        }

        // Resolve jobs
        // TODO(EhWhoAmI)
        // For now, we would have 100% of the population working, because we
        // haven't got to social simulation yet. But in the future, this will
        // probably have to change.
        auto& employee = universe.get_or_emplace<LaborInformation>(entity);
        employee.working_population = segment.population;
    }
}

namespace cqsp::common::systems {
void ProcessSettlement(Universe& universe, entity settlement, Market& market,
                       ResourceConsumption& marginal_propensity_base,
                       ResourceConsumption& autonomous_consumption_base, float savings) {
    // Get the transport cost
    auto& infrastructure = universe.get<infrastructure::CityInfrastructure>(settlement);
    // Calculate the infrastructure cost
    double infra_cost = infrastructure.default_purchase_cost - infrastructure.improvement;

    // Loop through the population segments through the settlements
    for (entt::entity segmententity : universe.get<Settlement>(settlement).population) {
        // Compute things
        PopulationSegment& segment = universe.get_or_emplace<PopulationSegment>(segmententity);
        ResourceConsumption& consumption = universe.get_or_emplace<ResourceConsumption>(segmententity);
        // Reduce pop to some unreasonably low level so that the economy can
        // handle it
        const uint64_t population = segment.population / 10;

        consumption = autonomous_consumption_base;

        // This value only changes when pop changes and
        // should be calculated in SysPopulationGrowth
        consumption *= population;

        Wallet& wallet = universe.get_or_emplace<Wallet>(segmententity);
        const double cost = (consumption * market.price).GetSum();
        wallet -= cost;    // Spend, even if it puts the pop into debt
        if (wallet > 0) {  // If the pop has cash left over spend it
            // Add to the cost of price of transport
            ResourceConsumption extraconsumption = marginal_propensity_base;
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
}  // namespace cqsp::common::systems

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
        const ConsumerGood& good = universe.get<ConsumerGood>(cgentity);
        marginal_propensity_base[cgentity] = good.marginal_propensity;
        autonomous_consumption_base[cgentity] = good.autonomous_consumption;
        savings -= good.marginal_propensity;
    }  // These tables technically never need to be recalculated
    //auto settlement
    //  = universe.view<Settlement>();

    // Loop through the settlements on a planet, then process the market?
    int settlement_count = 0;
    for (entt::entity entity : universe.view<Habitation>()) {
        // Get the children, because reasons
        // All planets with a habitation WILL have a market
        auto& market = universe.get_or_emplace<Market>(entity);
        // Read the segment information
        auto& habit = universe.get<Habitation>(entity);
        for (entt::entity settlement : habit.settlements) {
            ProcessSettlement(universe, settlement, market, marginal_propensity_base, autonomous_consumption_base,
                              savings);
            settlement_count++;
        }
    }
    SPDLOG_TRACE("Processing {} settlements in {} markets", settlement_count, market_view.size());
}
