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

namespace cqsp {
namespace common {
namespace components {
struct IndustrialZone {
    std::vector<entt::entity> industries;
};

enum ProductionType { factory, mine, service };

struct Production {
    //TODO(AGM) This is a hardcoded enum, move to a hjson
    ProductionType type;
    entt::entity recipe;
};

struct Factory {};

struct Mine {};

struct Service {};

struct Farm {
    // Farms have a harvest period?
    // Farms can fluctuate between how much they generate per tick.
    // Also add food good
};

struct RawResourceGen {};
}  // namespace components
}  // namespace common
}  // namespace cqsp
