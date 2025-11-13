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
    auto& orb = universe.get<Orbit>(body);
    if (orb.semi_major_axis == 0) {
        return;
    }
    const int res = 500;
    std::vector<glm::vec3> orbit_points;
    double SOI = std::numeric_limits<double>::infinity();
    if (universe.valid(orb.reference_body)) {
        SOI = universe.get<common::components::bodies::Body>(orb.reference_body).SOI;
    }

    orbit_points.reserve(res);
    // If hyperbolic
    if (orb.eccentricity > 1) {
        double v_inf = common::components::types::GetHyperbolicAsymptopeAnomaly(orb.eccentricity);
        // Remove one because it's slightly off.
        for (int i = 1; i < res; i++) {
            ZoneScoped;
            double theta = -v_inf * (1 - (double)i / (double)res) + v_inf * (((double)i / (double)res));

            glm::vec3 vec = common::components::types::toVec3(orb, theta);
            // Check if the length is greater than the SOI, then we don't add it
            if (glm::length(vec) < SOI) {
                orbit_points.push_back(vec);
            }
        }
    } else {
        for (int i = 0; i <= res; i++) {
            ZoneScoped;
            double theta = std::numbers::pi * 2 / res * i;

            glm::vec3 vec = common::components::types::toVec3(orb, theta);

            // If the length is greater than the sphere of influence, then
            // remove it
            if (glm::length(vec) < SOI) {
                orbit_points.push_back(vec);
            }
        }
    }

    // universe.remove<types::OrbitDirty>(body);
    auto& line = universe.get_or_emplace<components::OrbitMesh>(body);
    // Get the orbit line
    // Do the points
    line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
}
}  // namespace cqsp::client::systems
