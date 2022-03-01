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
#include "common/systems/population/cityinformation.h"

#include "common/components/surface.h"
#include "common/components/population.h"

uint64_t cqsp::common::systems::GetCityPopulation(Universe& universe,
                                                entt::entity city) {
    namespace cqspc = common::components;
    if (!universe.any_of<cqspc::Settlement>(city)) {
        return 0;
    }
    uint64_t pop_count = 0;
    auto& settlement = universe.get<cqspc::Settlement>(city);
    for (entt::entity pop : settlement.population) {
        pop_count += universe.get<cqspc::PopulationSegment>(pop).population;
    }
    return pop_count;
}
