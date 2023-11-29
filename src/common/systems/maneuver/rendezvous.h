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

#include <utility>

#include <glm/vec3.hpp>

#include "common/components/orbit.h"

namespace cqsp::common::systems {
// Tools for rendezvous
// Coplanar rendezvous
std::pair<glm::dvec3, double> CoplanarIntercept(const components::types::Orbit& start_orbit,
                                                const components::types::Orbit& end_orbit, double epoch);
}  // namespace cqsp::common::systems
