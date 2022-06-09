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
#include <string>

#include <entt/entt.hpp>

#include "common/components/units.h"

namespace cqsp::common::components::bodies {
struct Body {
    /*
     * Radius of the body
     */
    types::kilometer radius;

    entt::entity star_system;

    /// <summary>
    /// Radius of sphere of influence
    /// rsoi = a(m/M)^2/5
    /// </summary>
    types::kilometer SOI;
    types::kilogram mass;

    // gravitational constant in km^3 * s^-2
    double GM;

    // Rotation period in seconds
    double rotation;

    // Axial rotation
    double axial = 0.0;
};

struct TexturedTerrain {
    std::string terrain_name;
    std::string normal_name;
};

struct NautralObject {};
/// <summary>
/// An object for the children of an orbital object.
/// </summary>
struct OrbitalSystem {
    // Set the tree
    std::vector<entt::entity> children;
    void push_back(const entt::entity& entity) {
        children.push_back(entity);
    }
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
}  // namespace cqsp::common::components::bodies
