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

void cqsp::common::systems::SysPopulationGrowth::DoSystem(Game& game) {
    namespace cqspc = cqsp::common::components;
    Universe& universe = game.GetUniverse();

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
            universe.emplace_or_replace<cqspc::Hunger>(entity);
        } else {
            universe.remove_if_exists<cqspc::Hunger>(entity);
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

void cqsp::common::systems::SysPopulationConsumption::DoSystem(Game& game) {
    namespace cqspc = cqsp::common::components;
    Universe& universe = game.GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
        // The population will feed, I guess
        entt::entity good = universe.goods["consumer_good"];
        // So a consumer good is a kilogram, and the mass of a unit of consumer good is 6500 kg.
        // A person generated 4.9 pounds every day in the US according to the EPA.
        // Some of it is food, some it is other resources, but we don't need to have the nuance about
        // it yet. Since all things have to be thrown away, we'd have to assume that the generation of consumer
        // goods is the same as the consumption of consumer goods.
        // 4.9 pounds is roughly equal to 2.2226 kg, and divide it by 24 to get per tick, equals to 0.0926083333kg.
        uint64_t consumption = segment.population * 0.09261;
        universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[good] = consumption;
    }
}
