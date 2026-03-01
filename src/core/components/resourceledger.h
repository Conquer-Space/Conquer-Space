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
#include <map>
#include <string>
#include <vector>

namespace cqsp::core::components {
/**
* Class for goods to be accessed through an array
*/
enum class GoodEntity : uint32_t {};

class ResourceLedger;
class ResourceVector;

typedef std::map<GoodEntity, double> LedgerMap;

class ResourceMap : private LedgerMap {
 public:
    ResourceMap() = default;
    ~ResourceMap() = default;

    double operator[](const GoodEntity) const;

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

    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const ResourceLedger&);
    void operator/=(const ResourceLedger&);

    void operator-=(const ResourceVector&);
    void operator+=(const ResourceVector&);
    void operator*=(const ResourceVector&);
    void operator/=(const ResourceVector&);

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
    void MultiplyAdd(const ResourceVector&, double);

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

    bool HasGood(GoodEntity good) { return (*this).find(good) != (*this).end(); }

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

/// <summary>
/// This is a vector meant for fast iteration
/// </summary>
class ResourceVector : public std::vector<std::pair<GoodEntity, double>> {
    typedef std::pair<GoodEntity, double> LedgerPair;
    typedef std::vector<LedgerPair> LedgerVector;

 public:
    ResourceVector() = default;
    ~ResourceVector() = default;

    void operator-=(const ResourceMap&);
    void operator+=(const ResourceMap&);
    void operator*=(const ResourceMap&);
    void operator/=(const ResourceMap&);

    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const ResourceLedger&);
    void operator/=(const ResourceLedger&);

    void operator-=(const double value);
    void operator+=(const double value);
    void operator*=(const double value);
    void operator/=(const double value);

    ResourceVector operator-(const double value) const;
    ResourceVector operator+(const double value) const;
    ResourceVector operator*(const double value) const;
    ResourceVector operator/(const double value) const;

    ResourceVector operator+(const ResourceVector& value) const;
    ResourceVector operator*(const ResourceVector& value) const;

    /// <summary>
    /// Returns the average of all values in the ledger, with
    /// division by zero resulting in infiniy
    /// </summary>
    double Average();

    double Min();
    double Max();
    GoodEntity MinGood();
    GoodEntity MaxGood();

    void Finalize();

    bool HasGood(const GoodEntity& good) const {
        return std::find_if(begin(), end(), [good](const LedgerPair& _good) { return _good.first == good; }) != end();
    }

    double operator[](const GoodEntity& good) const {
        auto val = std::find_if(begin(), end(), [good](const LedgerPair& _good) { return _good.first == good; });
        if (val == end()) {
            return 0;
        } else {
            return val->second;
        }
    }

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
    double MultiplyAndGetSum(const ResourceLedger& other) const;

    bool contains(const GoodEntity& entity) const {
        return std::find_if(begin(), end(), [entity](const LedgerPair& _good) { return _good.first == entity; }) !=
               end();
    }

    // All the things that we get from map
    using LedgerVector::operator=;
    using LedgerVector::begin;
    using LedgerVector::cbegin;
    using LedgerVector::cend;
    using LedgerVector::clear;
    using LedgerVector::crbegin;
    using LedgerVector::crend;
    using LedgerVector::emplace;
    using LedgerVector::empty;
    using LedgerVector::end;
    using LedgerVector::erase;
    using LedgerVector::rbegin;
    using LedgerVector::rend;
    using LedgerVector::size;
};

class ResourceLedger {
 private:
    std::vector<double> ledger;

 public:
    explicit ResourceLedger(size_t count);

    operator ResourceMap() const;

    double operator[](const GoodEntity value) const;
    double& operator[](const GoodEntity value);

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

    void operator-=(const ResourceMap&);
    void operator+=(const ResourceMap&);
    void operator*=(const ResourceMap&);
    void operator/=(const ResourceMap&);

    void operator-=(const ResourceVector&);
    void operator+=(const ResourceVector&);
    void operator*=(const ResourceVector&);
    void operator/=(const ResourceVector&);

    void operator-=(const double value);
    void operator+=(const double value);
    void operator*=(const double value);
    void operator/=(const double value);

    ResourceLedger operator-(const ResourceLedger&) const;
    ResourceLedger operator+(const ResourceLedger&) const;
    ResourceLedger operator*(const ResourceLedger&) const;
    ResourceLedger operator/(const ResourceLedger&) const;

    ResourceLedger operator-(const ResourceMap&) const;
    ResourceLedger operator+(const ResourceMap&) const;
    ResourceLedger operator*(const ResourceMap&) const;
    ResourceLedger operator/(const ResourceMap&) const;

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

    /// <summary>
    /// All resources in this ledger are greater than the number
    /// </summary>
    bool operator>(const double&);

    /// <summary>
    /// All resources in this ledger are less than than the number
    /// </summary>
    bool operator<(const double&);

    bool operator==(const double&);
    bool operator!=(const double&);
    bool operator<=(const double&);
    bool operator>=(const double&);

    void AssignFrom(const ResourceLedger&);

    /** 
     * Transfers amount resoures into ledger_to
     */
    void TransferTo(ResourceLedger& ledger_to, const ResourceLedger& amount);
    // Equivalent to this += other * double
    void MultiplyAdd(const ResourceLedger&, double);

    // Add all the positive values in the other ledger to this ledger
    // Essentially this += other (if idx > 0)
    void AddPositive(const ResourceLedger&);

    // Add all the negative values in the other ledger to this ledger
    // Essentially this += abs(other) (if idx < 0)
    void AddNegative(const ResourceLedger&);

    /// <summary>
    /// Returns a copy of the vector with the values clamped between the min and max indicated
    /// </summary>
    ResourceLedger Clamp(const double, const double);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with division by zero resulting in infiniy
    /// </summary>
    ResourceLedger SafeDivision(const ResourceLedger&);

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with division by zero resulting in the specified value
    /// </summary>
    ResourceLedger SafeDivision(const ResourceLedger&, double);

    double MultiplyAndGetSum(const ResourceMap& other) const;
    double MultiplyAndGetSum(const ResourceLedger& other) const;

    /// <summary>
    /// Returns a copy of the vector divided by the indicated vector, with
    /// division by zero resulting in infiniy
    /// </summary>
    double Average() const;

    double Min() const;
    double Max() const;

    /**
     * Gets the sum of all the goods in this resource ledger.
     */
    double GetSum();

    auto begin() { return ledger.begin(); }

    auto end() { return ledger.end(); }

    size_t size() { return ledger.size(); }

    void clear();
};

ResourceMap CopyVals(const ResourceMap& keys, const ResourceMap& values);
}  // namespace cqsp::core::components
