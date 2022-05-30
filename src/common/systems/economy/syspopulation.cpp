/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/economy.h"


//Must be run after SysPopulationConsumption
//This is because population growht is dependent on if consumption was satisfied.
void cqsp::common::systems::SysPopulationGrowth::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
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
        // For now, we would have 100% of the population working, because we haven't got to social simulation
        // yet. But in the future, this will probably have to change.
        auto& employee = universe.get_or_emplace<cqspc::Employee>(entity);
        employee.working_population = segment.population;
    }
}



// In economics, the consumption function describes a relationship between consumption and disposable income.
//Its simplest form is the linear consumption function used frequently in simple Keynesian models
//For each consumer good consumption is modeled as follows
//C = a + b * Y
//C represents consumption of the consumer good
//a represents autonomous consumption that is independent of disposable income or when income levels are zero
//  if income levels cannot pay for this level of maintaince they are drawn from the population's savings or debt
//b represents the marginal propensity to consume or how much of their surplus income they will spend on that consumer good
//  Based on how many consumer goods they consume from this segment, we can find their economic strata.
//Y represents their disposable income (funds left over from last tick)
//  Population savings can be ensured by keeping the sum of all b values below 1
//Consumer Goods Include
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
void cqsp::common::systems::SysPopulationConsumption::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    cqspc::ResourceConsumption marginal_propensity_base;
    cqspc::ResourceConsumption autonomous_consumption_base;
    float savings = 1; //We calculate how much is saved since it is simpler than calculating spending
    for (entt::entity cgentity : universe.consumergoods) {
        const cqspc::ConsumerGood& good =
            universe.get_or_emplace<cqspc::ConsumerGood>(cgentity);
        marginal_propensity_base[cgentity] =
            good.marginal_propensity;
        marginal_propensity_base[cgentity] = good.autonomous_consumption;
        savings -= good.marginal_propensity;
    }// These tables technically never need to be recalculated
    auto popview = universe.view<cqspc::PopulationSegment>();
    for (auto [segmententity, segment] : popview.each()) 
    {
        cqspc::ResourceConsumption& consumption =
            universe.get_or_emplace<cqspc::ResourceConsumption>(segmententity);
        const uint64_t population = segment.population / 100000; // Reduce pop to some unreasonably low level so that the economy can handle it
        cqspc::Market& market = universe.get<cqspc::Market>(segmententity);
        consumption = autonomous_consumption_base;
        consumption *= population; //This value only changes when pop changes and should be calculated in SysPopulationGrowth
        cqspc::Wallet& wallet =
            universe.get_or_emplace<cqspc::Wallet>(segmententity);
        const double cost = (consumption * market.price).GetSum();
        wallet -= cost; //Spend, even if it puts the pop into debt
        if (wallet > 0) //If the pop has cash left over spend it
        {
            cqspc::ResourceConsumption extraconsumption =
                marginal_propensity_base;
            extraconsumption *= wallet;      //Distribute wallet amongst goods
            extraconsumption /= market.price;//Find out how much of each good you can buy 
            consumption += extraconsumption; //Remove purchased goods from the market
            wallet *= savings;               //Update savings
        }
        wallet += segment.population / 1000; //Inject cash
    }
}
