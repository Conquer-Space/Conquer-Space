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
/**
 * Transfers
 */
// This expects the altitude to be lower than the original orbit
components::Maneuver_t TransferFromBody(Universe& universe, const components::types::Orbit& orbit,
                                        const components::types::Kinematics& kinematics, double altitude) {
    // TODO(EhWhoAmI): Iterate in the future so that we get a more accurate departure angle
    // So we want this to happen:
    // Center Body ---- Orbiting Body ---- Satellite
    // Center Body ---- Orbiting Body
    //               |- Satellite
    Node orbiting_body_node(universe, orbit.reference_body);
    auto& orbiting_body_orbit = orbiting_body_node.get<components::types::Orbit>();

    // Now let's get our current velocity
    const double orbital_velocity = orbiting_body_orbit.OrbitalVelocity();
    const double orbital_radius = orbiting_body_orbit.GetOrbitingRadius();
    // So this should be the apoapsis and our new altitude should be an altitude
    const double intended_semi_major_axis = (orbital_radius + altitude) / 2;
    const double intended_eccentricity = components::types::GetEccentricity(orbital_radius, altitude);
    // Now let's get our expected velocity
    const double intended_orbital_velocity = components::types::OrbitVelocity(
        components::types::apoapsis, intended_eccentricity, intended_semi_major_axis, orbiting_body_orbit.GM);
    // Now we should get the delta
    // Now let's get our new velocity
    const double v_inf = orbital_velocity - intended_orbital_velocity;

    const double escape_eccentricity = 1 + (orbit.GetOrbitingRadius() * v_inf * v_inf) / orbit.GM;
    const double burn_angle = std::acos(-1 / escape_eccentricity);

    const double burn_amount = std::sqrt(v_inf * v_inf + 2 * orbit.GM / orbit.GetOrbitingRadius());
    // We need to figure out when we should burn
    // We get the vector of the forward, and it is that many degrees off that
    const auto& orbiting_kinematics = orbiting_body_node.get<components::types::Kinematics>();

    // Project the orbiting body's into the ship orbit plane.
    double time = 0;
    // Now create new kinematics
    const double v = GetBodyVelocityVectorInOrbitPlaneTrueAnomaly(orbit, kinematics, orbiting_kinematics);

    // Now we should add our true anomaly to this
    double target_true_anomaly = v - burn_angle + components::types::PI;
    time = orbit.TimeToTrueAnomaly(target_true_anomaly);
    //  + burn_angle - components::types::PI
    SPDLOG_TRACE("Time: {} v: {} burn angle: {} v_inf: {} eccentricity: {} burn: {}", time, v, burn_angle, v_inf,
                 escape_eccentricity, burn_amount);
    double initial_velocity = burn_amount - orbit.OrbitalVelocityAtTrueAnomaly(target_true_anomaly);
    return commands::MakeManeuver(glm::dvec3(0.0, initial_velocity, 0.0), time);
}

/*
* Get the velocity vector of the orbiting body in the orbital plane of the orbit, whose reference body is the orbiting body.
* 
* Projects the velocity vector onto the orbital plane.
* 
* @param orbit The orbit to define the orbital plane
* @param body_kinematics The kinematics of the orbiting bory
*/
glm::dvec3 GetBodyVelocityVectorInOrbitPlane(const components::types::Orbit& orbit,
                                             const components::types::Kinematics& body_kinematics) {
    const glm::dvec3 normal = components::types::GetOrbitNormal(orbit);
    const glm::dvec3 orbiting_forward_vector = glm::normalize(body_kinematics.velocity);
    // Project the orbiting body's into the ship orbit plane.
    glm::dvec3 vel_frame =
        orbiting_forward_vector - glm::dot(orbiting_forward_vector, normal) / glm::length2(normal) * normal;
    return vel_frame;
}

/**
* Gets the true anomaly of the velocity vector projected onto the body plane.
* 
* Projects the velocity vector onto the orbital plane, and gets the true anomaly of that with reference to the
* orbit.
* 
* @param orbit orbit to define the orbital plane
* @param kinematics kinematics of the orbit
* @param orbiting_kinematics Kinematics of the orbiting body
*/
double GetBodyVelocityVectorInOrbitPlaneTrueAnomaly(const components::types::Orbit& orbit,
                                                    const components::types::Kinematics& kinematics,
                                                    const components::types::Kinematics& orbiting_kinematics) {
    glm::dvec3 vel_frame = GetBodyVelocityVectorInOrbitPlane(orbit, orbiting_kinematics);

    // Eccentricity vector
    const glm::dvec3 ecc_v =
        components::types::GetEccentricityVector(kinematics.position, kinematics.velocity, orbit.GM);
    if (glm::length(ecc_v) == 0) {
        return 0;
    }
    double v = glm::angle(glm::normalize(ecc_v), glm::normalize(vel_frame));

    const glm::dvec3 normal = glm::normalize(glm::cross(kinematics.position, kinematics.velocity));
    glm::dvec3 cross_prod = glm::cross(glm::normalize(ecc_v), glm::normalize(vel_frame));
    if (glm::dot(cross_prod, normal) < 0) {
        v = components::types::TWOPI - v;
    }
    return v;
}
}  // namespace cqsp::common::systems
