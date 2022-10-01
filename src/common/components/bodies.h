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
#include <limits>

#include <entt/entt.hpp>

#include "common/components/units.h"

namespace cqsp::common::components::bodies {
struct Body {
    /*
     * Radius of the body
     */
    types::kilometer radius;

    /// <summary>
    /// Radius of sphere of influence
    /// rsoi = a(m/M)^2/5
    /// </summary>
    types::kilometer SOI = std::numeric_limits<double>::infinity();
    types::kilogram mass;

    // gravitational constant in km^3 * s^-2
    double GM;

    // Rotation period in seconds
    double rotation;

    // Axial rotation
    double axial = 0.0;

    double rotation_offset = 0.0;
};

/// <summary>
/// Calculates SOI
/// </summary>
/// Make sure the units match up
/// <param name="mass">Mass of planet/body to calculate</param>
/// <param name="reference_mass">Mass of sun/reference body</param>
/// <param name="sma">Semi major axis of the planet</param>
/// <returns>SOI of planet</returns>
inline double CalculateSOI(const double& mass, const double& reference_mass, const double& sma) {
    return sma * std::pow(mass / reference_mass, 0.4);
}

/// <summary>
/// Calculates mass from gravitational constant
/// </summary>
/// Masses of bodies are described in gravitational constant because it's more accurate to describe like that
/// <param name="GM"></param>
/// <returns></returns>
inline double CalculateMass(const double& GM) { return GM/types::G_km; }

/// <summary>
/// Calculates the current planet rotation angle
/// </summary>
/// <param name="time">current time in seconds</param>
/// <param name="day_length">length of day in seconds</param>
/// <param name="offset">offset of day</param>
/// <returns>Angle the planet should be in radians</returns>
inline double GetPlanetRotationAngle(const double& time, const double& day_length, const double& offset) {
    return (time / day_length - offset) * types::TWOPI;
}

struct TexturedTerrain {
    std::string terrain_name;
    std::string normal_name;
    std::string roughness_name;
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

struct DirtyOrbit {};

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
