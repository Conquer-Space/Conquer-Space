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
#include "client/scenes/universe/views/sysorbitgeometry.h"

#include <spdlog/spdlog.h>

#include <cmath>
#include <numbers>

#include "client/components/planetrendering.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "engine/graphics/primitives/polygon.h"
#include "tracy/Tracy.hpp"

namespace cqsp::client::systems {
using common::components::bodies::DirtyOrbit;
using common::components::types::Orbit;

SysOrbitGeometry::SysOrbitGeometry(common::Universe& universe) : universe(universe) {}

void SysOrbitGeometry::GenerateOrbitLines() {
    ZoneScoped;
    SPDLOG_TRACE("Creating planet orbits");

    // Generates orbits for satellites
    orbits_generated = 0;
    for (auto body : universe.view<Orbit>(entt::exclude<components::OrbitMesh>)) {
        GenerateOrbit(body);
        orbits_generated++;
    }

    // Generate dirty orbits
    for (auto body : universe.view<Orbit, DirtyOrbit>()) {
        GenerateOrbit(body);
        universe.remove<DirtyOrbit>(body);
        orbits_generated++;
    }
    // Delete unnecessary orbit
    for (entt::entity ship : universe.view<components::OrbitMesh, common::components::ships::Crash>()) {
        // Then delete the orbit
        universe.remove<components::OrbitMesh>(ship);
    }
}

void SysOrbitGeometry::GenerateOrbit(entt::entity body) {
    // Then produce orbits
    ZoneScoped;
    // Generate the orbit
    auto& orbit = universe.get<Orbit>(body);
    if (orbit.semi_major_axis == 0) {
        return;
    }

    double SOI = std::numeric_limits<double>::infinity();
    if (universe.valid(orbit.reference_body)) {
        SOI = universe.get<common::components::bodies::Body>(orbit.reference_body).SOI;
    }

    std::vector<glm::vec3> orbit_points =
        (orbit.eccentricity > 1) ? GenerateHyperbolicOrbit(orbit, SOI) : GenerateEllipticalOrbit(orbit, SOI);

    auto& line = universe.get_or_emplace<components::OrbitMesh>(body);
    // Get the orbit line
    // Do the points
    line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
}

std::vector<glm::vec3> SysOrbitGeometry::GenerateHyperbolicOrbit(const common::components::types::Orbit& orbit,
                                                                 double SOI) {
    std::vector<glm::vec3> orbit_points;
    orbit_points.reserve(ORBIT_RESOLUTION);
    double v_inf = common::components::types::GetHyperbolicAsymptopeAnomaly(orbit.eccentricity);
    // Remove one because it's slightly off.
    int points_generated = 0;
    for (int i = 1; i < ORBIT_RESOLUTION; i++) {
        ZoneScoped;
        double theta = std::lerp(-v_inf, v_inf, (double)i / (double)ORBIT_RESOLUTION);

        glm::vec3 vec = common::components::types::toVec3(orbit, theta);
        // Check if the length is greater than the SOI, then we don't add it
        if (glm::length(vec) < SOI) {
            orbit_points.push_back(vec);
            points_generated++;
        }
    }
    orbit_points.shrink_to_fit();
    return orbit_points;
}

std::vector<glm::vec3> SysOrbitGeometry::GenerateEllipticalOrbit(const common::components::types::Orbit& orbit,
                                                                 double SOI) {
    std::vector<glm::vec3> orbit_points;

    orbit_points.reserve(ORBIT_RESOLUTION);
    for (int i = 0; i <= ORBIT_RESOLUTION; i++) {
        ZoneScoped;
        double theta = std::numbers::pi * 2 / ORBIT_RESOLUTION * i;

        glm::vec3 vec = common::components::types::toVec3(orbit, theta);

        // If the length is greater than the sphere of influence, then
        // remove it
        if (glm::length(vec) < SOI) {
            orbit_points.push_back(vec);
        }
    }
    orbit_points.shrink_to_fit();
}
}  // namespace cqsp::client::systems
