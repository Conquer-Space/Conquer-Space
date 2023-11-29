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

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace cqsp::common::systems {
using namespace components::types;  // NOLINT
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

namespace {
double GetInclinationChangeDeltaV(const components::types::Orbit& orbit, double v, double d_inc) {
    double velocity = components::types::OrbitVelocity(v, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    double fpa = components::types::FlightPathAngle(orbit.eccentricity, v);
    return abs(2 * velocity * cos(fpa) * sin(d_inc / 2.));
}
}  // namespace

std::pair<glm::dvec3, double> SetInclination(const components::types::Orbit& orbit, double inclination) {
    double v_change = components::types::PI - orbit.w;
    const double d_inc = -1 * (inclination - orbit.inclination);
    // Figure out which v_change has the least delta-v expenditure
    double change_1 = GetInclinationChangeDeltaV(orbit, components::types::PI - orbit.w, d_inc);
    double change_2 = GetInclinationChangeDeltaV(orbit, 2 * components::types::PI - orbit.w, d_inc);
    // It will always be at apogee, so we're trying to find the point closest to apogee
    if (change_1 > change_2) {
        v_change = 2. * components::types::PI - orbit.w;
    }
    double v = components::types::OrbitVelocity(v_change, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    // Just set to original velocity
    // Delta-v should be at the ascending node
    // Do we have to get flight path angle?
    glm::dvec3 vector = glm::dvec3(0, v * (cos(d_inc) - 1), v * sin(d_inc));
    return std::make_pair(vector, orbit.TimeToMeanAnomaly(v_change));
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
    glm::dvec3 vector = glm::dvec3(0, v * (cos(d_inc) - 1), v * sin(d_inc));
    return std::make_pair(vector, orbit.TimeToMeanAnomaly(v_change));
}

// Shamelessly stolen from mechjeb
std::pair<glm::dvec3, double> MatchPlanes(const components::types::Orbit& orbit, const components::types::Orbit& dest) {
    // Find intersection theta with the other orbit
    double rel_inclination = dest.inclination - orbit.inclination;
    // Ascending node is 180-w?
    auto vec = glm::cross(GetOrbitNormal(orbit), GetOrbitNormal(dest));
    // Get the intersection of this orbit?
    auto projected = glm::proj(GetOrbitNormal(orbit), vec);
    auto periapsis = toVec3(orbit, 0);
    double angle = glm::angle(projected, periapsis);
    // If the vector points to the infalling part of the orbit then we need to do 2 pi minus the
    // angle from Pe to get the true anomaly. Test this by taking the the cross product of the
    // orbit normal and vector to the periapsis. This gives a vector that points to center of the
    // outgoing side of the orbit. If vectorToAN is more than 90 degrees from this vector, it occurs
    // during the infalling part of the orbit.
    //if (abs(glm::angle(GetOrbitNormal(orbit, ))))
    return std::pair<glm::dvec3, double>();
}
}  // namespace cqsp::common::systems
