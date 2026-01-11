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

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <entt/entt.hpp>

#include "core/components/area.h"
#include "core/components/resourceledger.h"
#include "core/components/units.h"

namespace cqsp::core::components {
/// <summary>
/// Something that has a mass.
/// </summary>
struct Matter {
    types::meter_cube volume;
    types::kilogram mass;
};

struct Energy {
    // Energy per unit
    types::joule energy;
};

/// <summary>
/// The unit name of the good. If it doesn't have it, then it's a quantity.
/// </summary>
struct Unit {
    std::string unit_name;
};

struct Good {};

/// <summary>
/// See SysPopulationConsumption for an explanation of these values
/// </summary>
struct ConsumerGood {
    ///   Autonomous consumption is independent of disposable income
    ///   or when income levels are zero
    ///   if income levels cannot pay for this level of maintenance they are
    ///   drawn from the population's savings or debt
    double autonomous_consumption;
    /// marginal propensity (demand) represents how
    ///   much of their surplus income they will spend on that consumer good
    ///   Based on how many consumer goods they consume from this segment, we
    ///   can find their economic strata.
    double marginal_propensity;
};

struct Mineral {};
// Good is for capital goods
struct CapitalGood {};

struct RecipeOutput {
    GoodEntity entity;
    double amount;
    ResourceMap operator*(const double value) const;
    ResourceMap operator*(ResourceMap&) const;
};

struct Recipe {
    ResourceMap input;
    RecipeOutput output;
    ProductionType type;
    float interval;
    // The actual factory worker amount is workers * productivity
    double workers;

    ResourceMap capitalcost;
};

struct RecipeCost {
    ResourceMap fixed;
    ResourceMap scaling;
};

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
};

struct CostBreakdown {
    double revenue;
    // How much it paid in materials to produce goods
    double materialcosts;
    // How much cash it took to maintain the factory
    double maintenance;
    // How much it paid to people
    double wages;
    double profit;
    // How much it paid in transport fees
    double transport;

    void Reset() {
        revenue = 0;
        materialcosts = 0;
        maintenance = 0;
        wages = 0;
        profit = 0;
        transport = 0;
    }
};

// TODO(AGM): Remove
struct FactoryTimer {
    float interval;
    float time_left;
};

struct ResourceConsumption : public ResourceMap {};
struct ResourceProduction : public ResourceMap {};

struct ResourceStockpile : public ResourceMap {};

struct FailedResourceTransfer {
    // Ledgers later to show how much
};

struct FailedResourceProduction {};

struct FailedResourceConsumption {};

struct ResourceDistribution {
    std::map<entt::entity, double> dist;
};
}  // namespace cqsp::core::components
