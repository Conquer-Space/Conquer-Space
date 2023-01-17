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

#include <entt/entt.hpp>

namespace cqsp {
namespace common {
namespace components {
struct Surface {
    int64_t seed;
};

/*
 * If people are living on a body or not
 */
struct Habitation {
    std::vector<entt::entity> settlements;
};

/*
 * People who live there.
 */
struct Settlement {
    std::vector<entt::entity> population;
};

struct TimeZone {
    double time_diff;
};

struct CityTimeZone {
    entt::entity time_zone;
};

struct Province {
    entt::entity country;
    std::vector<entt::entity> cities;
};

struct ProvinceColor {
    int r;
    int g;
    int b;

    bool operator==(const ProvinceColor& other) const { return (other.r == r && other.g == g && other.b == b); }

    operator int() const { return toInt(r, g, b); }

    static int toInt(int r, int g, int b) {
        int rgb = r;
        rgb = (rgb << 8) + g;
        rgb = (rgb << 8) + b;
        return rgb;
    }
};

// Capital city
struct CapitalCity {};
}  // namespace components
}  // namespace common
}  // namespace cqsp
