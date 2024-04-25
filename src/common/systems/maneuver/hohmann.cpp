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
#include "common/systems/maneuver/hohmann.h"

namespace components = cqsp::common::components;
namespace types = components::types;
using types::Orbit;
using types::OrbitVelocityAtR;
using types::GetCircularOrbitingVelocity;

namespace cqsp::common::systems {
HohmannPair_t UnsafeHohmannTransfer(const Orbit& orbit, double altitude) {
    const double new_sma = (orbit.semi_major_axis + altitude) / 2;
    const double new_velocity = OrbitVelocityAtR(orbit.GM, new_sma, orbit.semi_major_axis);
    const double old_velocity = GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    Maneuver_t start = std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), 0);

    // Set the time for the second orbit
    // The time to run it is half the period
    // Circularize at top orbit
    const double apogee_velocity = OrbitVelocityAtR(orbit.GM, new_sma, altitude);
    GetCircularOrbitingVelocity(orbit.GM, altitude);
    const double transfer_time = types::PI * sqrt(new_sma * new_sma * new_sma / orbit.GM);
    // Circularize
    const double final_velocity = types::GetCircularOrbitingVelocity(orbit.GM, altitude);
    Maneuver_t end = std::make_pair(glm::dvec3(0, final_velocity - apogee_velocity, 0), transfer_time);
    return std::make_pair(start, end);
}

std::optional<HohmannPair_t> HohmannTransfer(const Orbit& orbit, double altitude) {
    if (orbit.eccentricity > 1e-5) {
        // Can't do it
        return std::nullopt;
    }

    return std::make_optional(UnsafeHohmannTransfer(orbit, altitude));
}
}  // namespace cqsp::common::systems
