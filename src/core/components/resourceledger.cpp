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
#include "core/components/resourceledger.h"

#include <algorithm>
#include <limits>
#include <ranges>
#include <utility>

namespace cqsp::core::components {
namespace {
using cqsp::core::components::ResourceMap;
template <class Function>
ResourceMap merge_apply(const ResourceMap &m1, const ResourceMap &m2, ResourceMap::mapped_type identity,
                        Function func) {
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    auto comp = m1.value_comp();
    ResourceMap res;
    while (true) {
        bool end1 = it1 == m1.end();
        bool end2 = it2 == m2.end();
        if (end1 && end2) break;

        if (end2 || (!end1 && comp(*it1, *it2))) {
            res.emplace(it1->first, func(it1->second, identity));
            ++it1;
            continue;
        }
        if (end1 || comp(*it2, *it1)) {
            res.emplace(it2->first, func(identity, it2->second));
            ++it2;
            continue;
        }
        res.emplace(it1->first, func(it1->second, it2->second));
        ++it1;
        ++it2;
    }
    return res;
}

template <class Function>
bool MergeCompare(const ResourceMap &m1, const ResourceMap &m2, ResourceMap::mapped_type identity, Function func) {
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    auto comp = m1.value_comp();
    bool op = true;
    while (true) {
        bool end1 = it1 == m1.end();
        bool end2 = it2 == m2.end();
        if (end1 && end2) break;

        if (end2 || (!end1 && comp(*it1, *it2))) {
            // Compare
            op &= func(it1->second, identity);
            ++it1;
            continue;
        }
        if (end1 || comp(*it2, *it1)) {
            // Compare
            op &= func(identity, it2->second);
            ++it2;
            continue;
        }
        op &= func(it1->second, it2->second);
        ++it1;
        ++it2;
    }
    return op;
}
}  // namespace

using cqsp::core::components::ResourceMap;

double ResourceMap::operator[](const entt::entity entity) const {
    cqsp::core::components::LedgerMap::const_iterator location = this->find(entity);
    if (location == this->end()) {
        return 0;
    } else {
        return location->second;
    }
}

bool ResourceMap::EnoughToTransfer(const ResourceMap &amount) {
    bool b = true;
    for (auto it = amount.begin(); it != amount.end(); it++) {
        b &= (*this)[it->first] >= it->second;
    }
    return b;
}

void ResourceMap::AddPositive(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (iterator->second <= 0) continue;
        (*this)[iterator->first] += iterator->second;
    }
}

void ResourceMap::AddNegative(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (iterator->second >= 0) continue;
        (*this)[iterator->first] += -iterator->second;
    }
}

void ResourceMap::operator-=(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] -= iterator->second;
    }
}

void ResourceMap::operator+=(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] += iterator->second;
    }
}

void ResourceMap::operator*=(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] *= iterator->second;
    }
}

void ResourceMap::operator/=(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] /= iterator->second;
    }
}

void ResourceMap::operator-=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] -= value;
    }
}

void ResourceMap::operator+=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] += value;
    }
}

void ResourceMap::operator*=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] *= value;
    }
}

void ResourceMap::operator/=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] /= value;
    }
}

ResourceMap ResourceMap::operator+(const ResourceMap &other) const {
    ResourceMap ledger = *this;
    ledger += other;
    return ledger;
}

ResourceMap ResourceMap::operator-(const ResourceMap &other) const {
    ResourceMap ledger = *this;
    ledger -= other;
    return ledger;
}

ResourceMap ResourceMap::operator*(const ResourceMap &other) const {
    ResourceMap ledger = *this;
    ledger *= other;
    return ledger;
}

ResourceMap ResourceMap::operator/(const ResourceMap &other) const {
    ResourceMap ledger = *this;
    ledger /= other;
    return ledger;
}

ResourceMap ResourceMap::operator+(const double value) const {
    ResourceMap ledger = *this;
    ledger += value;
    return ledger;
}

ResourceMap ResourceMap::operator-(const double value) const {
    ResourceMap ledger = *this;
    ledger -= value;
    return ledger;
}

ResourceMap ResourceMap::operator*(const double value) const {
    ResourceMap ledger = *this;
    ledger *= value;
    return ledger;
}

ResourceMap ResourceMap::operator/(const double value) const {
    ResourceMap ledger = *this;
    ledger /= value;
    return ledger;
}

// Not sure if this is faster than a function, but wanted to have fun with the preprocessor,
// so here we go
#define compare(map, compare_to, comparison)                                   \
    bool op = true;                                                            \
    if ((map).empty()) {                                                       \
        return 0 comparison compare_to;                                        \
    }                                                                          \
    for (auto iterator = (map).begin(); iterator != (map).end(); iterator++) { \
        op &= iterator->second comparison compare_to;                          \
    }                                                                          \
    return op;

bool ResourceMap::operator>(const double &i) { compare((*this), i, >) }

bool ResourceMap::operator<(const double &i) { compare((*this), i, <) }

bool ResourceMap::operator==(const double &i) { compare((*this), i, ==) }

bool ResourceMap::operator<=(const double &i) { compare((*this), i, <=) }

bool ResourceMap::operator>=(const double &i) { compare((*this), i, >=) }

bool ResourceMap::operator>=(const ResourceMap &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a >= b; });
}

bool ResourceMap::LedgerEquals(const ResourceMap &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a == b; });
}

bool ResourceMap::operator<(const ResourceMap &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a < b; });
}

bool ResourceMap::operator>(const ResourceMap &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a > b; });
}

bool ResourceMap::operator<=(const ResourceMap &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a <= b; });
}

/**
 * Assigns values from the other ledger to this ledger.
 */
void ResourceMap::AssignFrom(const ResourceMap &ledger) {
    for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
        (*this)[iterator->first] = iterator->second;
    }
}

void ResourceMap::TransferTo(ResourceMap &ledger_to, const ResourceMap &amount) {
    for (auto iterator = amount.begin(); iterator != amount.end(); iterator++) {
        (*this)[iterator->first] -= iterator->second;
        ledger_to[iterator->first] += iterator->second;
    }
}

void ResourceMap::MultiplyAdd(const ResourceMap &other, double value) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] += iterator->second * value;
    }
}

void ResourceMap::RemoveResourcesLimited(const ResourceMap &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        double &t = (*this)[iterator->first];
        t -= iterator->second;
        if (t < 0) {
            t = 0;
        }
    }
}

ResourceMap ResourceMap::LimitedRemoveResources(const ResourceMap &other) {
    ResourceMap removed;
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        double &t = (*this)[iterator->first];
        if (t > iterator->second) {
            removed[iterator->first] = iterator->second;
            t -= iterator->second;
        } else {
            removed[iterator->first] = t;
            t = 0;
        }
    }
    return removed;
}

ResourceMap ResourceMap::UnitLedger(const double val) {
    ResourceMap newleg;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        newleg[iterator->first] = val;
    }
    return newleg;
}

ResourceMap ResourceMap::Clamp(const double minclamp, const double maxclamp) {
    ResourceMap newleg;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        double val = newleg[iterator->first];
        if (val > maxclamp) {
            val = maxclamp;
        } else if (val < minclamp) {
            val = minclamp;
        }
        newleg[iterator->first];
    }
    return newleg;
}

bool ResourceMap::HasAllResources(const ResourceMap &ledger) {
    if (&ledger == this) {
        return true;
    }
    return std::ranges::all_of(ledger, [this](auto led) { return (*this)[led.first] > 0; });
}

double ResourceMap::GetSum() {
    double t = 0;
    for (auto it = this->begin(); it != this->end(); it++) {
        t += it->second;
    }
    return t;
}

double ResourceMap::MultiplyAndGetSum(ResourceMap &other) {
    double sum = 0;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        sum += iterator->second * other[iterator->first];
    }
    return sum;
}

ResourceMap ResourceMap::SafeDivision(const ResourceMap &other) {
    ResourceMap ledger;
    ledger = *this;
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (iterator->second == 0) {
            ledger[iterator->first] = std::numeric_limits<double>::infinity();
        } else if (ledger[iterator->first] == 0) {
            ledger[iterator->first] = 0;
        } else {
            ledger[iterator->first] = ledger[iterator->first] / iterator->second;
        }
    }
    return ledger;
}

ResourceMap ResourceMap::SafeDivision(const ResourceMap &other, double value) {
    ResourceMap ledger;
    ledger = *this;
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (iterator->second == 0) {
            ledger[iterator->first] = value;
        } else if (ledger[iterator->first] == 0) {
            ledger[iterator->first] = 0;
        } else {
            ledger[iterator->first] = ledger[iterator->first] / iterator->second;
        }
    }
    return ledger;
}
/// <summary>
/// Finds the smallest value in the Ledger.
/// </summary>
/// <returns>The smallest value in the ledger, returns 0 if none found</returns>
double ResourceMap::Min() {
    if (this->begin() == this->end()) {
        return 0;
    }
    double minimum = this->begin()->second;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        if (iterator->second < minimum) minimum = iterator->second;
    }
    return minimum;
}

/// <summary>
/// Finds the largest value in the Ledger.
/// </summary>
/// <returns>The largest value in the ledger</returns>
double ResourceMap::Max() {
    double Maximum = this->begin()->second;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        if (iterator->second > Maximum) Maximum = iterator->second;
    }
    return Maximum;
}

double ResourceMap::Average() { return this->GetSum() / this->size(); }

std::string ResourceMap::to_string() {
    std::string str = "{";
    for (auto it = this->begin(); it != this->end(); it++) {
        str.append(" ");
        str.append(std::to_string(static_cast<std::uint32_t>(it->first)));
        str.append(",");
        str.append(std::to_string(it->second));
    }
    str.append("}");
    return str;
}

/// <summary>
/// Creates a new resource ledger using the keys from one resource ledger, and the values from annother
/// </summary>
ResourceMap CopyVals(const ResourceMap &keys, const ResourceMap &values) {
    ResourceMap tkeys = keys;
    for (auto iterator = keys.begin(); iterator != keys.end(); iterator++) {
        tkeys[iterator->first] = values[iterator->first];
    }
    return tkeys;
}

}  // namespace cqsp::core::components
