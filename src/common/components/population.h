/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#pragma once
#include "common/components/resource.h"

namespace cqsp::common::components {
struct PopulationSegment {
    uint64_t population;
    /// The total potential workers
    /// Everyone from the minimum working age (18) to the maximum age (65)
    /// The labor force must always be less than the population
    /// population - labor_force = dependents
    uint64_t labor_force;
};

struct Hunger {};
}  // namespace cqsp::common::components
