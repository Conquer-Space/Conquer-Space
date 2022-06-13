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

//See SysPopulationConsumption for an explanation of these values
struct ConsumerGood {
    double autonomous_consumption;
    double marginal_propensity;
};

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

    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const ResourceLedger&);
    void operator/=(const ResourceLedger&);
    void operator-=(const double value);
    void operator+=(const double value);
    void operator*=(const double value);
    void operator/=(const double value);

    ResourceLedger operator-(const ResourceLedger&) const;
    ResourceLedger operator+(const ResourceLedger&) const;
    ResourceLedger operator*(const ResourceLedger&) const;
    ResourceLedger operator/(const ResourceLedger&) const;
    ResourceLedger operator-(const double value) const;
    ResourceLedger operator+(const double value) const;
    ResourceLedger operator*(const double value) const;
    ResourceLedger operator/(const double value) const;

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

    /// <summary>
    /// Returns a copy of the vector with the values set to indicated value
    /// </summary>
    ResourceLedger UnitLeger(const double);

    /// <summary>
    /// Returns a copy of the vector with the values clamped between the min and max indicated
    /// </summary>
    ResourceLedger Clamp(const double, const double);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with division by zero resulting in infiniy
    /// </summary>
    ResourceLedger SafeDivision(const ResourceLedger&);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with
    /// division by zero resulting in infiniy
    /// </summary>
    double Average();


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
    using LedgerMap::size;
    using LedgerMap::mapped_type;


};
ResourceLedger ResourceLedgerZip(const ResourceLedger& key,const ResourceLedger& value);


struct Recipe {
    ResourceLedger input;
    ResourceLedger output;

    ProductionType type;

    float interval;
    double workers;

    ResourceLedger capitalcost;
};

struct RecipeCost {
    ResourceLedger fixed;
    ResourceLedger scaling;
};

//TODO(AGM): Remove ProductionTraits and FactoryProductivity
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

// Factory size
struct FactorySize {
    double size;
};

struct CostBreakdown {
    double profit;
    double materialcosts;
    double maintaince;
    double wages;
    double net;
};


//Multplier on prouduction
struct ProductionRatio {
    ResourceLedger input;
    ResourceLedger output;
};

//Essentially resource consumption + production
struct ResourceIO {
    ResourceLedger input;
    ResourceLedger output;
};

// TODO(AGM): Remove
struct FactoryTimer {
    float interval;
    float time_left;
};

//Resource generator 

struct ResourceConsumption : public ResourceLedger {};
struct ResourceProduction : public ResourceLedger {};

struct ResourceConverter {
    entt::entity recipe;
};

struct ResourceStockpile : public ResourceLedger { };



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
