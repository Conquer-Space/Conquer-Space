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
#include "core/actions/maneuver/basicmaneuver.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace cqsp::core::systems {
using namespace components::types;  // NOLINT
std::pair<glm::dvec3, double> CircularizeAtApoapsis(const Orbit& orbit) {
    // Get apogee
    const double new_velocity = GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    // Get velocity at apogee
    const double old_velocity = OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    double time = orbit.TimeToTrueAnomaly(PI);
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), time);
}

std::pair<glm::dvec3, double> CircularizeAtPeriapsis(const Orbit& orbit) {
    // Get apogee
    const double new_velocity = GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
    // Get velocity at apogee
    const double old_velocity = OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    // This should go retrograde, so it should be negative
    double time = orbit.TimeToTrueAnomaly(0);
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), time);
}

std::pair<glm::dvec3, double> SetApoapsis(const Orbit& orbit, double altitude) {
    const double new_sma = (orbit.GetPeriapsis() + altitude) / 2;
    double old_velocity = OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    double new_velocity = OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetPeriapsis());
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), orbit.TimeToTrueAnomaly(0));
}

std::pair<glm::dvec3, double> SetPeriapsis(const Orbit& orbit, double altitude) {
    if (orbit.eccentricity > 1.) {
        // Then give a null maneuver.
        return std::make_pair(glm::dvec3(0, 0, 0), 0);
    }
    const double new_sma = (orbit.GetApoapsis() + altitude) / 2;
    double old_velocity = OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    double new_velocity = OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
    return std::make_pair(glm::dvec3(0, new_velocity - old_velocity, 0), orbit.TimeToTrueAnomaly(PI));
}

namespace {
double GetInclinationChangeDeltaV(const Orbit& orbit, double v, double d_inc) {
    double velocity = OrbitVelocity(v, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    double fpa = FlightPathAngle(orbit.eccentricity, v);
    return abs(2 * velocity * cos(fpa) * sin(d_inc / 2.));
}
}  // namespace

std::pair<glm::dvec3, double> SetInclination(const Orbit& orbit, double inclination) {
    double v_change = PI - orbit.w;
    const double d_inc = -1 * (inclination - orbit.inclination);
    // Figure out which v_change has the least delta-v expenditure
    double change_1 = GetInclinationChangeDeltaV(orbit, PI - orbit.w, d_inc);
    double change_2 = GetInclinationChangeDeltaV(orbit, 2 * PI - orbit.w, d_inc);
    // It will always be at apogee, so we're trying to find the point closest to apogee
    if (change_1 > change_2) {
        v_change = 2. * PI - orbit.w;
    }
    double v = OrbitVelocity(v_change, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    // Just set to original velocity
    // Delta-v should be at the ascending node
    // Do we have to get flight path angle?
    glm::dvec3 vector = glm::dvec3(0, v * (cos(d_inc) - 1), v * sin(d_inc));
    return std::make_pair(vector, orbit.TimeToTrueAnomaly(v_change));
}

std::pair<glm::dvec3, double> SetCircularInclination(const Orbit& orbit, double inclination) {
    double v_change = PI - orbit.w;
    // Figure out if we're going into or out of the other orbital plane.
    double sign = -1;
    if (orbit.v > v_change) {
        v_change = 2 * PI - orbit.w;
        sign = 1;
    }
    if (orbit.w > PI) {
        sign *= -1;
    }
    const double d_inc = (inclination - orbit.inclination) * sign;
    double v = GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    // Just set to original velocity
    // Delta-v should be at the ascending node
    glm::dvec3 vector = glm::dvec3(0, v * (cos(d_inc) - 1), v * sin(d_inc));
    return std::make_pair(vector, orbit.TimeToTrueAnomaly(v_change));
}

/*
* Computes maneuver to match
* Note: This expects the orbit to 
*/
std::pair<glm::dvec3, double> MatchPlanes(const Orbit& orbit, const Orbit& target) {
    // Now compute the location we need to execute the impulse
    double taoan = AscendingTrueAnomaly(orbit, target);
    double time_to_true_anomaly = orbit.TimeToTrueAnomaly(taoan);

    // Get orbit velocity at point to maintain orbit
    double v = OrbitVelocity(taoan, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    // Calculate delta-v vector
    double target_ta = GetTrueAnomaly(target, time_to_true_anomaly);
    double theta = glm::angle(glm::normalize(GetOrbitNormal(orbit)), glm::normalize(GetOrbitNormal(target)));
    glm::dvec3 vector = glm::dvec3(0, v * (cos(theta) - 1), v * sin(theta));
    return std::make_pair(vector, orbit.TimeToTrueAnomaly(taoan));
}
}  // namespace cqsp::core::systems
