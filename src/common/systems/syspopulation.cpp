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
#include "common/systems/syspopulation.h"

#include <spdlog/spdlog.h>
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/economy.h"

void cqsp::common::systems::SysPopulationGrowth::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
        // If it's hungry, decay population
        if (universe.all_of<cqspc::Hunger>(entity)) {
            // Population decrease will be about 1 percent each year.
            float increase = 1.f - static_cast<float>(Interval()) * 0.00000114077116f;
            segment.population *= increase;  // NOLINT(bugprone-narrowing-conversions): don't know how to fix it
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
            segment.population *= increase; // NOLINT(bugprone-narrowing-conversions)
        }

        // Resolve jobs
        // TODO(EhWhoAmI)
        // For now, we would have 100% of the population working, because we haven't got to social simulation
        // yet. But in the future, this will probably have to change.
        auto& employee = universe.get_or_emplace<cqspc::Employee>(entity);
        employee.working_population = segment.population;
    }
}

void cqsp::common::systems::SysPopulationConsumption::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
        // The population will feed, I guess
        entt::entity good = universe.goods["consumer_good"];

        // Reduce it to some unreasonably low level so that the economy can handle it
        uint64_t consumption = segment.population/100000;
        universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[good] = static_cast<double>(consumption);

        // Inject some cash into the population segment, so that they don't run out of money to buy the stuff
        auto& wallet = universe.get_or_emplace<cqspc::Wallet>(entity);
        wallet += static_cast<double>(segment.population / 1000); // NOLINT(bugprone-integer-division)
    }
}
