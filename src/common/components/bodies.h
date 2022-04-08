/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include <map>
#include <tuple>

#include <entt/entt.hpp>

#include "common/components/units.h"

namespace cqsp {
namespace common {
namespace components {
namespace bodies {
struct Body {
    /*
     * Radius of the body
     */
    types::kilometer radius;

    entt::entity star_system;
    Body();
    Body(entt::entity _star_system, types::kilometer _radius = 0);
};

struct Terrain {
    int seed;
    entt::entity terrain_type;
};

struct TerrainData {
    float sea_level;
    std::map<float, std::tuple<int, int, int, int>> data;
};

struct Star {};

struct Planet {};

struct LightEmitter {};
/*
 * A collection of stuff
 */
struct StarSystem {
    std::vector<entt::entity> bodies;
};
}  // namespace bodies
}  // namespace components
}  // namespace common
}  // namespace cqsp
