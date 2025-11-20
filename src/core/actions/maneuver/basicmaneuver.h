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
#pragma once

#include <utility>

#include <glm/vec3.hpp>

#include "core/components/maneuver.h"
#include "core/components/orbit.h"

// Basic satellite maneuvers
namespace cqsp::core::systems {
// Creates a manuever for a circular orbit at apogee, the new altitude of the entire orbit will be the apogee,
// the maneuever will be at apogee
components::Maneuver_t CircularizeAtApoapsis(const components::types::Orbit& orbit);
// Creates a manuever for a circular orbit at perigee, new altitude will be perigee, the maneuver will be at the perigee
components::Maneuver_t CircularizeAtPeriapsis(const components::types::Orbit& orbit);

// Raises or lowers apogee to the new altitude. This will not check if you are colliding with the body :(
// The burn will be at the periapsis
components::Maneuver_t SetApoapsis(const components::types::Orbit& orbit, double altitude);
// Raises or lowers perapsis to the new altitude. This will not check if you are colliding with the body
// The burn will be at the apoapsis
components::Maneuver_t SetPeriapsis(const components::types::Orbit& orbit, double altitude);
// This will find the nearest point in the inclination that intersects with the orbital inclination plane 0 degrees.
components::Maneuver_t SetInclination(const components::types::Orbit& orbit, double inclination);
components::Maneuver_t SetCircularInclination(const components::types::Orbit& orbit, double inclination);
components::Maneuver_t MatchPlanes(const components::types::Orbit& orbit, const components::types::Orbit& target);
}  // namespace cqsp::core::systems
