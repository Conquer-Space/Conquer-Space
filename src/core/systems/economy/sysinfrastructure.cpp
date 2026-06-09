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
#include "core/systems/economy/sysinfrastructure.h"

#include <tracy/Tracy.hpp>

#include "core/components/area.h"
#include "core/components/infrastructure.h"
#include "core/components/labor.h"

namespace cqsp::core::systems {

namespace infrastructure = components::infrastructure;

void InfrastructureSim::DoSystem() {
    ZoneScoped;
    // Process construction sectors...
    // Add jobs
    const double tick_hours = (40. / static_cast<double>(components::StarDate::WEEK)) * Interval();
    entt::entity default_job = GetUniverse().default_job;
    // then get the consumer good
    auto& labor = GetUniverse().get<components::Labor>(default_job);
    for (auto&& [entity, construction_sector, market] :
         GetUniverse().view<infrastructure::ConstructionSector, components::Market>().each()) {
        // Also automatically adjust construction sector...
        // Add to the job market or something
        // For each we also add a week of work
        if (construction_sector.construction_capacity == 0) {
            continue;
        }
        components::ResourceVector vec;
        vec.emplace_back(labor.good,
                         static_cast<double>(construction_sector.construction_capacity) * tick_hours * 1000);
        auto [cost, tax] = market.PurchaseFromMarket(vec);
        // Then if it's zeroo we just ignore it?
        construction_sector.construction_cost = (cost + tax) / construction_sector.construction_capacity;
    }
}
}  // namespace cqsp::core::systems
