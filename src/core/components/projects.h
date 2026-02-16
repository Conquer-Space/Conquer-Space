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

#include <cstdint>
#include <vector>

namespace cqsp::core::components {
enum class ProjectType { Design, Manufacturing, Integration };

struct Project {
    // Current progress
    uint64_t progress;

    // The final progress in days or something
    uint64_t max_progress;
    // Then we also attach some sort of resource ledger for the consumption
    // Also somehow
    double project_total_cost = 0;
    double project_last_cost = 0;

    ProjectType type;
    entt::entity result;
};
}  // namespace cqsp::core::components
