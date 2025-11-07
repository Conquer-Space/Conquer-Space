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
#include "common/actions/population/cityinformation.h"

#include "common/components/population.h"
#include "common/components/surface.h"

namespace cqsp::common::actions {
uint64_t GetCityPopulation(Node& city) {
    if (!city.any_of<components::Settlement>()) {
        return 0;
    }
    uint64_t pop_count = 0;
    auto& settlement = city.get<components::Settlement>();
    for (Node pop : city.Convert(settlement.population)) {
        pop_count += pop.get<components::PopulationSegment>().population;
    }
    return pop_count;
}
}  // namespace cqsp::common::actions