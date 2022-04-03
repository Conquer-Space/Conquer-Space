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

#include <entt/entt.hpp>

namespace cqsp {
namespace common {
namespace components {
namespace infrastructure {
// Infrastrucutre provides many benefits
// Electric grid, roads, railways, water supply, etc.
struct Infrastructure {
    // Different types of infrastructure w
};

struct CityInfrastructure {};

struct PowerPlant {
    double production;
};

struct PowerConsumption {
    double max;
    double min;
    double current;
};

struct CityPower {
    double total_power_prod;
    double total_power_consumption;
};

struct BrownOut {};

class SpacePort {};
}  // namespace infrastructure
}  // namespace components
}  // namespace common
}  // namespace cqsp
