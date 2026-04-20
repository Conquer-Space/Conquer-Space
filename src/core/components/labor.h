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

#include <utility>
#include <vector>

#include <entt/entity/entity.hpp>

#include "core/components/resourceledger.h"

namespace cqsp::core::components {
struct Labor {
    GoodEntity good;
    // Then some other stats like uh things
};

struct PopulationLabor {
    // This will be updated less regularly
    std::vector<std::pair<entt::entity, uint64_t>> labor_distribution;
    // A pop sells its labor hours on the market
    ResourceVector labor_hours;
};
}  // namespace cqsp::core::components
