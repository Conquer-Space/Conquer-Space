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

std::pair<glm::dvec3, double> SetInclination(const components::types::Orbit& orbit, double inclination) {
    const double d_inc = inclination - orbit.inclination;
    double dv = (2 * sin(d_inc / 2) * (1 + orbit.eccentricity * cos(orbit.v)) * orbit.nu * orbit.semi_major_axis) /
                (sqrt(1 - orbit.eccentricity * orbit.eccentricity) * cos(orbit.w + orbit.v));
    return std::make_pair(glm::dvec3(), 0);
}

std::pair<glm::dvec3, double> SetCircularInclination(const components::types::Orbit& orbit, double inclination) {
    double v_change = components::types::PI - orbit.w;
    // Figure out if we're going into or out of the other orbital plane.
    double sign = -1;
    if (orbit.v > v_change) {
        v_change = 2 * components::types::PI - orbit.w;
        sign = 1;
    }
    if (orbit.w > components::types::PI) {
        sign *= -1;
    }
    const double d_inc = (inclination - orbit.inclination) * sign;
    double v = components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    // Just set to original velocity
    // Delta-v should be at the ascending node
    glm::dvec3 vector =
        glm::dvec3(0, v * (cos(d_inc) - 1),
                   v * sin(d_inc));  //glm::dquat(glm::dvec3(inclination, 0, 0)) * glm::dvec3(0, 0, delta_v);
    return std::make_pair(vector, orbit.TimeToMeanAnomaly(v_change));
}
}  // namespace cqsp::common::systems
