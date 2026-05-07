/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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
#include "core/systems/economy/syslabormarket.h"

#include <tracy/Tracy.hpp>

#include "core/components/labor.h"
#include "core/components/market.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
using components::Market;

void SysLaborMarket::DoSystem() {
    ZoneScoped;
    auto market_view = GetUniverse().view<Market, components::Settlement>(entt::exclude<components::PlanetaryMarket>);
    const double tick_hours = (40. / static_cast<double>(components::StarDate::WEEK)) * Interval();
    for (const auto&& [entity, market, settlement] : market_view.each()) {
        // Then do some processing or something
        // So we should weight our goods and see if we're lacking our production to come up with
        // a push and pull thing
        // Then we need an inverse way
        settlement.job_demands.clear();
        settlement.job_demands.reserve(labor_goods.size());
        for (const components::GoodEntity good : labor_goods) {
            // If this is positive
            // So anything positive are the jobs that we want to get rid of?
            settlement.job_demands.emplace_back(good_to_labor_map[good],
                                                (market.supply[good] - 2 * market.demand[good]) / tick_hours);
        }
    }
}

void SysLaborMarket::Init() {
    auto labor_view = GetUniverse().view<components::LaborGood>();
    for (entt::entity entity : labor_view) {
        labor_goods.push_back(GetUniverse().good_map[entity]);
    }

    auto job_view = GetUniverse().view<components::Labor>();
    for (auto&& [entity, labor] : job_view.each()) {
        good_to_labor_map[labor.good] = entity;
    }
}
}  // namespace cqsp::core::systems
