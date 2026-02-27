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

#include <string>
#include <vector>

#include <entt/entt.hpp>

namespace cqsp::core::components {
/// <summary>
/// The list of cities and settlements that are on a city
/// </summary>
struct Settlements {
    std::vector<entt::entity> settlements;
    std::vector<entt::entity> provinces;
};

enum class ColonizationSteps { Surveying, Preparation, InitialBase, HumanSettlement, Expansion, PermanentSettlement };

struct ColonizationTarget {
    // TODO(EhWhoAmI): This should be a vector or something as multiple entities might want to colonize a specific
    // place
    entt::entity colonizer;
    ColonizationSteps steps;
    // Progress in percentage or something
    int progress;
};

struct City {
    entt::entity province;
};

struct ProvincedPlanet {
    // Identifier for the mouse to identify which province the person has clicked on
    std::string province_map;
    std::string province_definitions;
    std::string adjacencies;
};

/// <summary>
/// A settlement is a place where a population can live in, such as a city or a space station
/// </summary>
struct Settlement {
    std::vector<entt::entity> population;
    // Infrastructure and other things
    // we might need air supply and other things like that
};

struct TimeZone {
    double time_diff = 0;
};

struct CityTimeZone {
    entt::entity time_zone = entt::null;
};

struct Province {
    entt::entity country = entt::null;
    std::vector<entt::entity> cities;
    std::vector<entt::entity> neighbors;
};

struct ProvinceColor {
    int r;
    int g;
    int b;
    ProvinceColor() = default;
    ProvinceColor(int r, int g, int b) : r(r), g(g), b(b) {}

    bool operator==(const ProvinceColor& other) const { return (other.r == r && other.g == g && other.b == b); }

    operator int() const { return toInt(r, g, b); }

    static int toInt(int r, int g, int b) {
        int rgb = r;
        rgb = (rgb << 8) + g;
        rgb = (rgb << 8) + b;
        return rgb;
    }

    static ProvinceColor fromInt(int color) {
        int r = (color & 0xFF0000) >> 16;
        int g = (color & 0x00FF00) >> 8;
        int b = (color & 0x0000FF);

        return ProvinceColor(r, g, b);
    }
};

// Capital city
struct CapitalCity {};

struct DockedShips {
    std::vector<entt::entity> docked_ships;
};
}  // namespace cqsp::core::components
