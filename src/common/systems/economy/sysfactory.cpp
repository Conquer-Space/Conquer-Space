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

    #include "common/components/area.h"
    #include "common/components/economy.h"
    #include "common/util/profiler.h"
    #include <spdlog/spdlog.h>  


    void cqsp::common::systems::SysProduction::DoSystem() {
        Universe& universe = GetUniverse();
        auto view = universe.view<components::Industry>();
        BEGIN_TIMED_BLOCK(INDUSTRY);
        int factories = 0;
        for (auto [industryentity, industry] : view.each()) 
        {
            // Get market attached, get sd ratio for the goods it produces, then
            // adjust production.
            //


            components::Market& market =
                universe.get_or_emplace<components::Market>(industryentity);
            for (entt::entity productionentity : industry.industries) {
                //Industries MUST have production and a linked recipe
                if (!universe.all_of<components::Production>(
                        productionentity))
                    continue;
                components::Recipe recipe =
                    universe.get_or_emplace<components::Recipe>(
                        universe
                        .get<components::Production>(productionentity)
                        .recipe);
            
                if (!universe.all_of<components::FactorySize>(
                        productionentity)) {
                    components::FactorySize& newsize =
                        universe.get_or_emplace<components::FactorySize>(
                            productionentity);
                    components::ProductionRatio& newratio =
                        universe.get_or_emplace<components::ProductionRatio>(
                            productionentity);
                    newsize.size = 1000;

                }

                components::FactorySize& size =
                    universe.get_or_emplace<components::FactorySize>(
                        productionentity);
                components::ProductionRatio& ratio =
                    universe.get_or_emplace<components::ProductionRatio>(
                    productionentity);

                components::ResourceLedger input =
                    (recipe.input * ratio.input) +
                    (recipe.capitalcost * (0.01 * size.size));
                components::ResourceLedger output = recipe.output * ratio.output;


                components::CostBreakdown& costs =
                    universe.get_or_emplace<components::CostBreakdown>(
                        productionentity);
                costs.maintaince =
                    (recipe.capitalcost * market.price).GetSum() * 0.01 *
                    size.size;
                costs.materialcosts =
                    (recipe.input * ratio.input * market.price).GetSum();
                costs.profit =
                    (recipe.output * ratio.output * market.price).GetSum();
                costs.wages = size.size * 1000 * 50000;
                costs.net = costs.profit - costs.maintaince -
                            costs.materialcosts - costs.wages;
                if (costs.net > 0) {
                    size.size *= 1.02;
                    
                } else {
                    size.size *= 0.99;
                }
                market.demand += input;
                market.supply += output;
                ratio.input = recipe.input.UnitLeger(size.size);
                ratio.output = recipe.output.UnitLeger(size.size); 

                factories += 1;
            }

        }
        END_TIMED_BLOCK(INDUSTRY);
        //SPDLOG_INFO("Updating {} factories", factories);
    }
