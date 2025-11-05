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
#include "common/actions/maneuver/transfers.h"

#include <spdlog/spdlog.h>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "common/actions/maneuver/commands.h"

namespace cqsp::common::systems {
// This expects the altitude to be lower than the original orbit
components::Maneuver_t TransferFromBody(Universe& universe, const components::types::Orbit& orbit,
                                        const components::types::Kinematics& kinematics, double altitude) {
    // So we want this to happen:
    // Center Body ---- Orbiting Body ---- Satellite
    // Center Body ---- Orbiting Body
    //               |- Satellite
    Node orbiting_body_node(universe, orbit.reference_body);
    auto& orbiting_body_orbit = orbiting_body_node.get<components::types::Orbit>();

    // Now let's get our current velocity
    double orbital_velocity = orbiting_body_orbit.OrbitalVelocity();
    double orbital_radius = orbiting_body_orbit.GetOrbitingRadius();
    // So this should be the apoapsis and our new altitude should be an altitude
    double intended_semi_major_axis = (orbital_radius + altitude) / 2;
    double intended_eccentricity = components::types::GetEccentricity(orbital_radius, altitude);
    // Now let's get our expected velocity
    double intended_orbital_velocity = components::types::OrbitVelocity(
        components::types::apoapsis, intended_eccentricity, intended_semi_major_axis, orbiting_body_orbit.GM);
    // Now we should get the delta
    // Now let's get our new velocity
    double v_inf = orbital_velocity - intended_orbital_velocity;

    double escape_eccentricity = 1 + (orbit.GetOrbitingRadius() * v_inf * v_inf) / orbit.GM;
    double burn_angle = std::acos(-1 / escape_eccentricity);

    double burn_amount = std::sqrt(v_inf * v_inf + 2 * orbit.GM / orbit.GetOrbitingRadius());
    // We need to figure out when we should burn
    // We get the vector of the forward, and it is that many degrees off that
    auto& orbiting_kinematics = orbiting_body_node.get<components::types::Kinematics>();
    glm::dvec3 normal = components::types::GetOrbitNormal(orbit);
    glm::dvec3 orbiting_forward_vector = glm::normalize(orbiting_kinematics.velocity);
    // Project the orbiting body's into the ship orbit plane.
    glm::dvec3 vel_frame =
        orbiting_forward_vector - glm::dot(orbiting_forward_vector, normal) / glm::length2(normal) * normal;

    // Now we should get the eccentricity vector
    const auto h = glm::cross(kinematics.position, kinematics.velocity);
    // Eccentricity vector
    const glm::dvec3 ecc_v = glm::cross(kinematics.velocity, h) / orbit.GM - glm::normalize(kinematics.position);
    if (glm::length(ecc_v) == 0) {
        // Then do stuff
    }
    double v = glm::angle(glm::normalize(ecc_v), glm::normalize(vel_frame));
    // It's probably this that's causing our issues
    assert(!isnan(v));
    if (v < 0) {
        v += components::types::PI;
    }
    // Now we should add our true anomaly to this
    double time = orbit.TimeToTrueAnomaly(v + burn_angle - components::types::PI);

    SPDLOG_INFO("Time: {} v: {} burn angle: {} v_inf: {} eccentricity: {} burn: {}", time, v, burn_angle, v_inf,
                escape_eccentricity, burn_amount);

    double initial_velocity = burn_amount - orbit.OrbitalVelocityAtTrueAnomaly(v + burn_angle);
    return commands::MakeManeuver(glm::dvec3(0.0, initial_velocity, 0.0), time);
}
}  // namespace cqsp::common::systems
