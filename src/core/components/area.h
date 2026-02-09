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

#include <entt/entt.hpp>

namespace cqsp::core::components {
struct IndustrialZone {
    std::vector<entt::entity> industries;
};

struct Construction {
    int progress;
    int maximum;
    int levels;
};

enum class ProductionType { factory, mine, service };

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

// Factory size
struct IndustrySize {
    // The size of the factory.
    // the maximum output of the factory is
    double size;
    // The utilization of the factory.
    double utilization;
    double diff;
    double diff_delta;

    int workers;
    bool shortage = false;
    double wages = 25;
    int continuous_losses = 0;
    int continuous_gains = 0;
};

struct CostBreakdown {
    double revenue;
    // How much it paid in materials to produce goods
    double material_costs;
    // How much cash it took to maintain the factory
    double maintenance;
    // How much it paid to people
    double wages;
    double profit;
    // How much it paid in transport fees
    double transport;

    double amount_sold = 0;

    void Reset() {
        revenue = 0;
        material_costs = 0;
        maintenance = 0;
        wages = 0;
        profit = 0;
        amount_sold = 0;
        transport = 0;
    }
};
}  // namespace cqsp::core::components
