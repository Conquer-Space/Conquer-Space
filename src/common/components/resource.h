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

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <entt/entt.hpp>

#include "common/components/units.h"
#include "common/components/area.h"

namespace cqsp {
namespace common {
namespace components {
/// <summary>
/// Something that has a mass.
/// </summary>
struct Matter {
    cqsp::common::components::types::meter_cube volume;
    cqsp::common::components::types::kilogram mass;
};

struct Energy {
    // Energy per unit
    cqsp::common::components::types::joule energy;
};

/// <summary>
/// The unit name of the good. If it doesn't have it, then it's a quantity.
/// </summary>
struct Unit {
    std::string unit_name;
};

struct Good {};

struct Mineral {};

typedef std::map<entt::entity, double> LedgerMap;

class ResourceLedger : private LedgerMap {
 public:
    ResourceLedger() = default;
    ~ResourceLedger() = default;

    /// <summary>
    /// This resource ledger has enough resources inside to transfer "amount" amount of resources away
    /// </summary>
    /// <param name="amount">Other resource ledger</param>
    /// <returns></returns>
    bool EnoughToTransfer(const ResourceLedger& amount);

    ResourceLedger operator-(const ResourceLedger&);
    ResourceLedger operator+(const ResourceLedger&);
    ResourceLedger operator*(double value);

    /// <summary>
    /// Multiplies the resource with the resource value in other ledger
    /// </summary>
    /// <param name=""></param>
    ResourceLedger operator*(ResourceLedger&);

    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const double value);

    /// <summary>
    /// Multiplies the resource with the resource value in other ledger
    /// </summary>
    /// <param name=""></param>
    void operator*=(ResourceLedger&);

    /// <summary>
    /// All resources in this ledger are smaller than than the other ledger
    /// </summary>
    bool operator<(const ResourceLedger&);

    /// <summary>
    /// All resources in this ledger are greater than the other ledger
    /// </summary>
    bool operator>(const ResourceLedger&);

    /// <summary>
    /// All resources in this ledger are smaller than or equal to than the other ledger
    /// </summary>
    bool operator<=(const ResourceLedger&);

    /// <summary>
    /// All resources in this ledger are greater than or equal to the other ledger
    /// </summary>
    bool operator>=(const ResourceLedger&);

    bool operator==(const ResourceLedger&);

    /// <summary>
    /// All resources in this ledger are greater than the number
    /// </summary>
    bool operator>(const double&);

    /// <summary>
    /// All resources in this ledger are less than than the number
    /// </summary>
    bool operator<(const double&);

    bool operator==(const double&);

    bool operator<=(const double&);
    bool operator>=(const double&);

    void AssignFrom(const ResourceLedger&);

    void TransferTo(ResourceLedger&, const ResourceLedger&);
    // Equivalant to this += other * double
    void MultiplyAdd(const ResourceLedger&, double);

    /// <summary>
    /// Removes the resources, and if the amount of resources removed are more than the resources
    /// inside the stockpile, it will set that resource to zero.
    /// </summary>
    /// <param name=""></param>
    void RemoveResourcesLimited(const ResourceLedger&);

    /// <summary>
    /// Same as RemoveResourcesLimited, except that it returns how much resources
    /// it took out.
    /// </summary>
    ResourceLedger LimitedRemoveResources(const ResourceLedger&);

    bool HasGood(entt::entity good) {
        return (*this).find(good) != (*this).end();
    }

    double GetSum();

    /// <summary>
    /// Multiplies the numbers stated in the resource ledger. Used for calculating the price, becuase
    /// usually the resource ledger will be the price.
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    double MultiplyAndGetSum(ResourceLedger& ledger);

    std::string to_string();

    // All the things that we get from map
    using LedgerMap::operator[];
    using LedgerMap::begin;
    using LedgerMap::end;
    using LedgerMap::cbegin;
    using LedgerMap::cend;
    using LedgerMap::crbegin;
    using LedgerMap::crend;
    using LedgerMap::rbegin;
    using LedgerMap::rend;
    using LedgerMap::clear;
    using LedgerMap::empty;
    using LedgerMap::emplace;
    using LedgerMap::value_comp;
    using LedgerMap::mapped_type;

#ifdef TRACY_ENABLE
    // Debug value to determine how many stockpile operations are done in the tick
    static int stockpile_additions;
#endif  // TRACY_ENABLE
};

struct Recipe {
    ResourceLedger input;
    ResourceLedger output;

    ProductionType type;

    float interval;
};

struct RecipeCost {
    ResourceLedger fixed;
    ResourceLedger scaling;
};

struct ProductionTraits {
    double max_production;
    double current_production;
};

/// <summary>
/// The multiplier of recipes the factory is generating right now. This is the amount of recipes the factory wants to
/// generate, or the production target.
/// </summary>
struct FactoryProductivity {
    // The amount they want to make now
    double current_production;
    // The physical limitation the factory can produce
    double max_production;
    // Other modifiers go here, I guess
    // Idk if i want a map, but that may not be a bad idea
};

struct FactoryTimer {
    float interval;
    float time_left;
};

struct ResourceGenerator : public ResourceLedger {};

struct ResourceConsumption : public ResourceLedger {};

struct ResourceConverter {
    entt::entity recipe;
};

struct ResourceStockpile : public ResourceLedger { };

struct ResourceDemand : public ResourceLedger { };

struct FailedResourceTransfer {
    // Ledgers later to show how much
};

struct FailedResourceProduction {};

struct FailedResourceConsumption {};

struct ResourceDistribution {
    std::map<entt::entity, double> dist;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
