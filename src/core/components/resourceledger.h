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

#include <map>
#include <string>

#include <entt/entt.hpp>

namespace cqsp::core::components {
typedef std::map<entt::entity, double> LedgerMap;

class ResourceMap : private LedgerMap {
 public:
    ResourceMap() = default;
    ~ResourceMap() = default;

    double operator[](const entt::entity) const;

    /// <summary>
    /// This resource ledger has enough resources inside to transfer "amount" amount of resources away
    /// </summary>
    /// <param name="amount">Other resource ledger</param>
    /// <returns></returns>
    bool EnoughToTransfer(const ResourceMap& amount);

    void operator-=(const ResourceMap&);
    void operator+=(const ResourceMap&);
    void operator*=(const ResourceMap&);
    void operator/=(const ResourceMap&);
    void operator-=(const double value);
    void operator+=(const double value);
    void operator*=(const double value);
    void operator/=(const double value);

    ResourceMap operator-(const ResourceMap&) const;
    ResourceMap operator+(const ResourceMap&) const;
    ResourceMap operator*(const ResourceMap&) const;
    ResourceMap operator/(const ResourceMap&) const;
    ResourceMap operator-(const double value) const;
    ResourceMap operator+(const double value) const;
    ResourceMap operator*(const double value) const;
    ResourceMap operator/(const double value) const;

    /// <summary>
    /// All resources in this ledger are smaller than than the other ledger
    /// </summary>
    bool operator<(const ResourceMap&);

    /// <summary>
    /// All resources in this ledger are greater than the other ledger
    /// </summary>
    bool operator>(const ResourceMap&);

    /// <summary>
    /// All resources in this ledger are smaller than or equal to than the other ledger
    /// </summary>
    bool operator<=(const ResourceMap&);

    /// <summary>
    /// All resources in this ledger are greater than or equal to the other ledger
    /// </summary>
    bool operator>=(const ResourceMap&);

    bool LedgerEquals(const ResourceMap&);

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

    void AssignFrom(const ResourceMap&);

    void TransferTo(ResourceMap&, const ResourceMap&);
    // Equivalant to this += other * double
    void MultiplyAdd(const ResourceMap&, double);

    /// <summary>
    /// Removes the resources, and if the amount of resources removed are more than the resources
    /// inside the stockpile, it will set that resource to zero.
    /// </summary>
    /// <param name=""></param>
    void RemoveResourcesLimited(const ResourceMap&);

    /// <summary>
    /// Same as RemoveResourcesLimited, except that it returns how much resources
    /// it took out.
    /// </summary>
    ResourceMap LimitedRemoveResources(const ResourceMap&);

    /// <summary>
    /// Returns a copy of the vector with the values set to indicated value
    /// </summary>
    ResourceMap UnitLedger(const double);

    // Add all the positive values in the other ledger to this ledger
    // Essentially this += other (if idx > 0)
    void AddPositive(const ResourceMap&);

    // Add all the negative values in the other ledger to this ledger
    // Essentially this += abs(other) (if idx < 0)
    void AddNegative(const ResourceMap&);

    /// <summary>
    /// Returns a copy of the vector with the values clamped between the min and max indicated
    /// </summary>
    ResourceMap Clamp(const double, const double);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with division by zero resulting in infiniy
    /// </summary>
    ResourceMap SafeDivision(const ResourceMap&);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with division by zero resulting in the specified value
    /// </summary>
    ResourceMap SafeDivision(const ResourceMap&, double);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with
    /// division by zero resulting in infiniy
    /// </summary>
    double Average();

    double Min();
    double Max();

    /// <summary>
    /// Checks if this current resource ledger has any resources in this list
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    bool HasAllResources(const ResourceMap&);

    bool HasGood(entt::entity good) { return (*this).find(good) != (*this).end(); }

    /**
     * Gets the sum of all the goods in this resource ledger.
     */
    double GetSum();

    /// <summary>
    /// Multiplies the numbers stated in the resource ledger. Used for calculating the price, becuase
    /// usually the resource ledger will be the price.
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    double MultiplyAndGetSum(ResourceMap& other);

    std::string to_string();

    // All the things that we get from map
    using LedgerMap::operator[];
    using LedgerMap::operator=;
    using LedgerMap::begin;
    using LedgerMap::cbegin;
    using LedgerMap::cend;
    using LedgerMap::clear;
    using LedgerMap::contains;
    using LedgerMap::crbegin;
    using LedgerMap::crend;
    using LedgerMap::emplace;
    using LedgerMap::empty;
    using LedgerMap::end;
    using LedgerMap::erase;
    using LedgerMap::mapped_type;
    using LedgerMap::rbegin;
    using LedgerMap::rend;
    using LedgerMap::size;
    using LedgerMap::value_comp;
};

ResourceMap CopyVals(const ResourceMap& keys, const ResourceMap& values);

}  // namespace cqsp::core::components
