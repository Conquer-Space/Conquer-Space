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

#include <entt/entity/entity.hpp>

namespace cqsp::core::components {
enum class ColonizationSteps { Surveying, Preparation, InitialBase, HumanSettlement, PermanentSettlement };

struct ColonizationTarget {
    // TODO(EhWhoAmI): This should be a vector or something as multiple entities might want to colonize a specific
    // place
    entt::entity colonizer;
    ColonizationSteps steps = ColonizationSteps::Surveying;
    // Progress in percentage or something
    int progress = 0;
};

struct Colony {
    // Since our population is likely to be very small, we should probably just have a resource ledger representing our
    uint16_t population;
    uint16_t max_population;  // The capacity that our colony can store
    // The different stuff the different parts it will provide
    std::vector<entt::entity> components;
};

struct HabitationModule {
    uint16_t crew;
};

struct Survey {};
}  // namespace cqsp::core::components
