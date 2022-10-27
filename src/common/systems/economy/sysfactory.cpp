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
#include "common/systems/economy/sysfactory.h"

#include <spdlog/spdlog.h>

#include <tracy/Tracy.hpp>

#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/util/profiler.h"
#include "common/components/surface.h"
#include "common/components/name.h"

namespace cqsp::common::systems {
namespace cqspc = cqsp::common::components;
namespace {
/// <summary>
/// Runs the production cycle
/// Consumes material from the market based on supply and then sells the manufactured goods on the market.
/// </summary>
/// <param name="universe">Registry used for searching for components</param>
/// <param name="entity">Entity containing an Inudstries that need to be processed</param>
/// <param name="market">The market the industry uses.</param>
void ProcessIndustries(common::Universe& universe, entt::entity entity,
                    cqspc::Market& market) {
    // Get the transport cost
    auto& infrastructure =
        universe.get<cqspc::infrastructure::CityInfrastructure>(entity);
    // Calculate the infrastructure cost
    double infra_cost =
        infrastructure.default_purchase_cost - infrastructure.improvement;

    auto& industries = universe.get<cqspc::IndustrialZone>(entity);
    for (entt::entity productionentity : industries.industries) {
        // Process imdustries
        // Industries MUST have production and a linked recipe
        if (!universe.all_of<components::Production>(productionentity))
            continue;
        components::Recipe recipe =
            universe.get_or_emplace<components::Recipe>(
                universe.get<components::Production>(productionentity)
                    .recipe);
        components::IndustrySize& size =
            universe.get_or_emplace<components::IndustrySize>(
                productionentity, 1000.0);
        // Calculate resource consumption
        components::ResourceLedger capitalinput =
            recipe.capitalcost * (0.01 * size.size);
        components::ResourceLedger input =
            (recipe.input + size.size) + capitalinput;

        // Calculate the greatest possible production
        components::ResourceLedger output;  // * ratio.output;
        output[recipe.output.entity] =
            recipe.output.amount * size.size * 10;

        // Figure out what's throttling production and maintaince
        double limitedinput =
            CopyVals(input, market.history.back().sd_ratio).Min();
        double limitedcapitalinput =
            CopyVals(capitalinput, market.history.back().sd_ratio)
                .Min();

        // Log how much manufacturing is being throttled by input
        market[recipe.output.entity].inputratio = limitedinput;

        if (limitedinput < 1) {  // If an input good is undersupplied on
                                    // the market, throttle production

            input *= limitedinput;
            output *= limitedinput;
            // Industry
            if (market.history.back().sd_ratio[recipe.output.entity] >
                1.1) {
                size.size *= 0.99;
            } else {  // Scale production

                if (limitedcapitalinput > 1) limitedcapitalinput = 1;
                if (market.history.back()
                        .sd_ratio[recipe.output.entity] > 1.1)
                    size.size *= 0.99;
                else
                    size.size *=
                        1 + (0.01) * std::fmin(limitedcapitalinput, 1);
            }
            market.demand += input;
            market.supply += output;

            double output_transport_cost = output.GetSum() * infra_cost;
            double input_transport_cost = input.GetSum() * infra_cost;
            // Next time need to compute the costs along with input and
            // output so that the factory doesn't overspend. We sorta
            // need a balanced economy
            components::CostBreakdown& costs =
                universe.get_or_emplace<components::CostBreakdown>(
                    productionentity);

            // Maintainence costs will still have to be upkept, so if
            // there isnt any resources to upkeep the place, then stop
            // the production
            costs.maintenance =
                (recipe.capitalcost * market.price).GetSum() * 0.01 *
                size.size;
            costs.materialcosts =
                (recipe.input * size.size * market.price).GetSum();
            costs.profit = (recipe.output * market.price).GetSum();
            costs.wages = size.size * 1000 * 50000;
            costs.net = costs.profit - costs.maintenance -
                        costs.materialcosts - costs.wages;
            costs.transport =
                output_transport_cost + input_transport_cost;
            if (costs.net > 0) {
                // size.size *= 1.02;
            } else {
                // size.size *= 0.99;
            }

            // ratio.ratio = recipe.input.UnitLeger(size.size);
            // ratio.output = recipe.output.UnitLeger(size.size);
        }
    }
} 
}// namespace cqspc
            
void SysProduction::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();
    auto view = universe.view<components::IndustrialZone>();
    BEGIN_TIMED_BLOCK(INDUSTRY);
    int factories = 0;
    // Loop through the markets
    auto market_view = universe.view<cqspc::Habitation>();
    int settlement_count = 0;
    for (entt::entity entity : market_view) {
        auto& market = universe.get_or_emplace<cqspc::Market>(entity);
        // Read the segment information
        // Get the children of the market
        auto& habitation = universe.get<cqspc::Habitation>(entity);
        for (entt::entity settlement : habitation.settlements) {
            ProcessIndustries(universe, settlement, market);
        }
    }
    END_TIMED_BLOCK(INDUSTRY);
    SPDLOG_TRACE("Updated {} factories, {} industries", factories,
                    view.size());

}
}  // namespace cqsp::common::systems
