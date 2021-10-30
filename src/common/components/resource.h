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

namespace cqsp {
namespace common {
namespace components {
struct Good {
    cqsp::common::types::meter_cube volume;
    cqsp::common::types::kilogram mass;
};

struct Mineral {};

struct Food { };
struct ResourceLedger : public std::map<entt::entity, double> {
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

    bool HasGood(entt::entity good) {
        return (*this).find(good) != (*this).end();
    }

    double GetSum();
    double MultiplyAndGetSum(ResourceLedger&);

    std::string to_string();
};

struct Recipe {
    ResourceLedger input;
    ResourceLedger output;

    float interval;
};

/// <summary>
/// The multiplier of recipes the factory is generating right now.
/// </summary>
struct FactoryProductivity {
    // Amount generated per generation
    float productivity;
};

/// <summary>
/// The maximum of multiples of recipes the factories can generate.
/// </summary>
struct FactoryCapacity {
    float capacity;
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

struct ResourceStockpile : public ResourceLedger {};

struct ResourceDemand : public ResourceLedger {};

struct FailedResourceTransfer {
    // Ledgers later to show how much
};

struct FailedResourceProduction {};

struct FailedResourceConsumption {};

struct ResourceDistribution : public std::map<entt::entity, double> {};
}  // namespace components
}  // namespace common
}  // namespace cqsp
