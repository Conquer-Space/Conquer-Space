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
#include <spdlog/spdlog.h>

#include "common/systems/economy/sysfactory.h"
#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/util/profiler.h"
#include "common/components/surface.h"
namespace cqsp::common::systems {
namespace cqspc = cqsp::common::components;
namespace {
void ProcessIndustries(common::Universe& universe, entt::entity entity,
                       cqspc::Market& market) {
    auto& industries = universe.get<cqspc::Industry>(entity);
    for (entt::entity productionentity : industries.industries) {
        // Process imdustries
        // Industries MUST have production and a linked recipe
        if (!universe.all_of<components::Production>(productionentity))
            continue;
        components::Recipe recipe = universe.get_or_emplace<components::Recipe>(
            universe.get<components::Production>(productionentity).recipe);
        components::FactorySize& size =
            universe.get_or_emplace<components::FactorySize>(productionentity,
                                                             1000.0);
        components::ProductionRatio& ratio =
            universe.get_or_emplace<components::ProductionRatio>(
                productionentity);

        components::ResourceLedger input =
            (recipe.input * ratio.input) +
            (recipe.capitalcost * (0.01 * size.size));
        components::ResourceLedger output = recipe.output * ratio.output;
        market.demand += input;
        market.supply += output;

        components::CostBreakdown& costs =
            universe.get_or_emplace<components::CostBreakdown>(
                productionentity);
        costs.maintenance =
            (recipe.capitalcost * market.price).GetSum() * 0.01 * size.size;
        costs.materialcosts =
            (recipe.input * ratio.input * market.price).GetSum();
        costs.profit = (recipe.output * ratio.output * market.price).GetSum();
        costs.wages = size.size * 1000 * 50000;
        costs.net = costs.profit - costs.maintenance - costs.materialcosts -
                    costs.wages;
        if (costs.net > 0) {
            size.size *= 1.02;
        } else {
            size.size *= 0.99;
        }

        ratio.input = recipe.input.UnitLeger(size.size);
        ratio.output = recipe.output.UnitLeger(size.size);
    }
}
}  // namespace

void SysProduction::DoSystem() {
    Universe& universe = GetUniverse();
    auto view = universe.view<components::Industry>();
    BEGIN_TIMED_BLOCK(INDUSTRY);
    int factories = 0;
// Loop through the markets
    auto market_view = universe.view<cqspc::Habitation> ();
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
    SPDLOG_TRACE("Updated {} factories, {} industries", factories, view.size());
}
}  // namespace cqsp::common::systems
