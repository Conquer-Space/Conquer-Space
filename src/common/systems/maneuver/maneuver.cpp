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
#include "common/systems/maneuver/maneuver.h"

namespace cqsp::common::systems {
std::pair<glm::dvec3, double> CircularizeAtApoapsis(const components::types::Orbit& orbit) {
    // Get apogee
    const double new_velocity = components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    // Get velocity at apogee
    const double old_velocity =
        components::types::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0),
                          orbit.TimeToMeanAnomaly(common::components::types::PI));
}

std::pair<glm::dvec3, double> CircularizeAtPeriapsis(const components::types::Orbit& orbit) {
    // Get apogee
    const double new_velocity = components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
    // Get velocity at apogee
    const double old_velocity =
        components::types::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    // This should go retrograde, so it should be negative
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), orbit.TimeToMeanAnomaly(0));
}

std::pair<glm::dvec3, double> SetApoapsis(const components::types::Orbit& orbit, double altitude) {
    const double new_sma = (orbit.GetPeriapsis() + altitude) / 2;
    double old_velocity = components::types::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    double new_velocity = components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetPeriapsis());
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), orbit.TimeToMeanAnomaly(0));
}

std::pair<glm::dvec3, double> SetPeriapsis(const components::types::Orbit& orbit, double altitude) {
    const double new_sma = (orbit.GetApoapsis() + altitude) / 2;
    double old_velocity = components::types::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    double new_velocity = components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0),
                          orbit.TimeToMeanAnomaly(common::components::types::PI));
}
}  // namespace cqsp::common::systems
