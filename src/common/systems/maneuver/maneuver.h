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

// Basic satellite maneuvers
namespace cqsp::common::systems {
// Creates a manuever for a circular orbit at apogee, the new altitude of the entire orbit will be the apogee,
// the maneuever will be at apogee
std::pair<glm::dvec3, double> CircularizeAtApoapsis(const components::types::Orbit& orbit);
// Creates a manuever for a circular orbit at perigee, new altitude will be perigee, the maneuver will be at the perigee
std::pair<glm::dvec3, double> CircularizeAtPeriapsis(const components::types::Orbit& orbit);

// Raises or lowers apogee to the new altitude. This will not check if you are colliding with the body :(
// The burn will be at the periapsis
std::pair<glm::dvec3, double> SetApoapsis(const components::types::Orbit& orbit, double altitude);
// Raises or lowers perapsis to the new altitude. This will not check if you are colliding with the body
// The burn will be at the apoapsis
std::pair<glm::dvec3, double> SetPeriapsis(const components::types::Orbit& orbit, double altitude);
std::pair<glm::dvec3, double> SetInclination(const components::types::Orbit& orbit, double inclination);
std::pair<glm::dvec3, double> SetCircularInclination(const components::types::Orbit& orbit, double inclination);
}  // namespace cqsp::common::systems
