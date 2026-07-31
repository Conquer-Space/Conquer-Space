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

#include <entt/entity/entity.hpp>

#include "core/components/resourceledger.h"
#include "core/components/vectormap.h"

namespace cqsp::core::components {
struct Need {
    // Then how much and what goods fulfill the need?
    // How much priority this will get versus other needs, the higher the more
    // priority?
    uint16_t priority = 0;
    // Minimum needed for like survivalB, everyone will try to hit that level
    // maybe
    uint16_t minimum;
    uint16_t minimum_desired;
    // Goods that can fulfill a certain need
    ResourceVector fulfillment_goods;
    GoodEntity default_good;
};

struct PopulationConsumption {
    VectorMap<entt::entity, double> needs;
    VectorMap<entt::entity, ResourceVector> consumption;
};
}  // namespace cqsp::core::components
