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

void cqsp::common::systems::SysMine::DoSystem() {
    auto view = GetUniverse().view<components::Mine, components::ResourceGenerator, components::MarketAgent>();
    for (const entt::entity& entity : view) {
        // Get market attached, get sd ratio for the goods it produces, then adjust production.
        //
        auto& gen = GetUniverse().get<components::ResourceGenerator>(entity);
        entt::entity generated = entt::null;
        double amount_generated = 0;
        auto gen_it = gen.begin();
        generated = gen_it->first;
        amount_generated = gen_it->second;
        auto& market = GetUniverse().get<components::Market>(
            GetUniverse().get<components::MarketAgent>(entity).market);

        // Reduce production because costs
        const double sd_ratio = market.GetSDRatio(generated);
        if (sd_ratio > 1) {
            // Reduce production
            auto& prod = GetUniverse().get_or_emplace<components::FactoryProductivity>(entity);
            // Inject some randomness to adjust the price
            prod.current_production *= 0.95;
        }

        if (sd_ratio < 1) {
            // Reduce production
            auto& prod = GetUniverse().get_or_emplace<components::FactoryProductivity>(entity);
            // Inject some randomness?
            prod.current_production *= 1.05;
        }
    }
}
