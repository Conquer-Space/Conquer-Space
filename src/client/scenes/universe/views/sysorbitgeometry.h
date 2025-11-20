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

#include <vector>

#include <glm/glm.hpp>

#include "core/components/orbit.h"
#include "core/universe.h"

namespace cqsp::client::systems {
class SysOrbitGeometry {
 public:
    SysOrbitGeometry(core::Universe&);
    void GenerateOrbitLines();

    int GetOrbitsGenerated() { return orbits_generated; }

 private:
    const int ORBIT_RESOLUTION = 500;
    int orbits_generated;
    core::Universe& universe;

    void GenerateOrbit(entt::entity body);
    std::vector<glm::vec3> GenerateHyperbolicOrbit(const core::components::types::Orbit& orbit, double SOI);
    std::vector<glm::vec3> GenerateEllipticalOrbit(const core::components::types::Orbit& orbit, double SOI);
};
}  // namespace cqsp::client::systems
