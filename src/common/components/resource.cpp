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
#include "common/components/resource.h"

#include <spdlog/spdlog.h>

#include <utility>
#include <limits>

namespace cqsp::common::components {
namespace {
using cqsp::common::components::ResourceLedger;
template<class Function>
ResourceLedger merge_apply(const ResourceLedger &m1, const ResourceLedger &m2,
                           ResourceLedger::mapped_type identity, Function func) {
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    auto comp = m1.value_comp();
    ResourceLedger res;
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

template<class Function>
bool MergeCompare(const ResourceLedger &m1, const ResourceLedger&m2,
                  ResourceLedger::mapped_type identity, Function func) {
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

using cqsp::common::components::ResourceLedger;

const double ResourceLedger::operator[](const entt::entity entity) const {
    cqsp::common::components::LedgerMap::const_iterator location = this->find(entity);
    if (location == this->end()) {
        return 0;
    } else {
        return location->second;
    }
}

bool ResourceLedger::EnoughToTransfer(const ResourceLedger &amount) {
    bool b = true;
    for (auto it = amount.begin(); it != amount.end(); it++) {
        b &= (*this)[it->first] >= it->second;
    }
    return b;
}

void ResourceLedger::operator-=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] -= iterator->second;
    }
}

void ResourceLedger::operator+=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] += iterator->second;
    }
}

void ResourceLedger::operator*=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] *= iterator->second;
    }
}

void ResourceLedger::operator/=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] /= iterator->second;
    }
}

void ResourceLedger::operator-=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] -= value;
    }
}

void ResourceLedger::operator+=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] += value;
    }
}


void ResourceLedger::operator*=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] *= value;
    }
}

void ResourceLedger::operator/=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] /= value;
    }
}

ResourceLedger ResourceLedger::operator+(const ResourceLedger &other) const {
    ResourceLedger ledger = *this;
    ledger += other;
    return ledger;
}

ResourceLedger ResourceLedger::operator-(const ResourceLedger &other) const {
    ResourceLedger ledger = *this;
    ledger -= other;
    return ledger;
}

ResourceLedger ResourceLedger::operator*(const ResourceLedger &other) const {
    ResourceLedger ledger = *this;
    ledger *= other;
    return ledger;
}

ResourceLedger ResourceLedger::operator/(const ResourceLedger &other) const {
    ResourceLedger ledger = *this;
    ledger /= other;
    return ledger;
}

ResourceLedger ResourceLedger::operator+(const double value) const {
    ResourceLedger ledger = *this;
    ledger += value;
    return ledger;
}

ResourceLedger ResourceLedger::operator-(const double value) const {
    ResourceLedger ledger = *this;
    ledger -= value;
    return ledger;
}

ResourceLedger ResourceLedger::operator*(const double value) const {
    ResourceLedger ledger = *this;
    ledger *= value;
    return ledger;
}

ResourceLedger ResourceLedger::operator/(const double value) const {
    ResourceLedger ledger = *this;
    ledger /= value;
    return ledger;
}

// Not sure if this is faster than a function, but wanted to have fun with the preprocessor,
// so here we go
#define compare(map, compare_to, comparison) \
    bool op = true; \
    if (map.size() == 0) { \
        return 0 comparison compare_to; \
    } \
    for (auto iterator = map.begin(); iterator != map.end(); iterator++) { \
        op &= iterator->second comparison compare_to; \
    } \
    return op;

bool ResourceLedger::operator>(const double &i) {
    compare((*this), i, >)
}

bool ResourceLedger::operator<(const double & i) {
    compare((*this), i, <)
}

bool ResourceLedger::operator==(const double &i) {
    compare((*this), i, ==)
}

bool ResourceLedger::operator<=(const double &i) {
    compare((*this), i, <=)
}

bool ResourceLedger::operator>=(const double &i) {
    compare((*this), i, >=)
}

bool ResourceLedger::operator>=(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a >= b; });
}

bool ResourceLedger::operator==(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a == b; });
}

bool ResourceLedger::operator<(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a < b; });
}

bool ResourceLedger::operator>(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a > b; });
}

bool ResourceLedger::operator<=(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a <= b; });
}

void ResourceLedger::AssignFrom(const ResourceLedger &ledger) {
    for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
        (*this)[iterator->first] = iterator->second;
    }
}

void ResourceLedger::TransferTo(ResourceLedger& ledger_to, const ResourceLedger & amount) {
    for (auto iterator = amount.begin(); iterator != amount.end(); iterator++) {
        (*this)[iterator->first] -= iterator->second;
        ledger_to[iterator->first] += iterator->second;
    }
}

void ResourceLedger::MultiplyAdd(const ResourceLedger & other, double value) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        (*this)[iterator->first] += iterator->second * value;
    }
}

void ResourceLedger::RemoveResourcesLimited(const ResourceLedger & other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        double &t = (*this)[iterator->first];
         t -= iterator->second;
         if (t < 0) {
             t = 0;
         }
    }
}

ResourceLedger ResourceLedger::LimitedRemoveResources(const ResourceLedger& other) {
    ResourceLedger removed;
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        double &t = (*this)[iterator->first];
        if (t > iterator-> second) {
            removed[iterator->first] = iterator->second;
            t -= iterator->second;
        } else {
            removed[iterator->first] = t;
            t = 0;
        }
    }
    return std::move(removed);
}

ResourceLedger ResourceLedger::UnitLeger(const double val) {
    ResourceLedger newleg;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        newleg[iterator->first] = val;
    }
    return newleg;
}

ResourceLedger ResourceLedger::Clamp(const double minclamp,
                                     const double maxclamp) {
    ResourceLedger newleg;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        double val = newleg[iterator->first];
        if (val > maxclamp)
            val = maxclamp;
        else if (val < minclamp)
            val = minclamp;
        newleg[iterator->first];
    }
    return newleg;
}

bool ResourceLedger::HasAllResources(const ResourceLedger & ledger) {
    for (auto led : ledger) {
        if ((* this)[led.first] <= 0) {
            return false;
        }
    }
    return true;
}

double ResourceLedger::GetSum() {
    double t = 0;
    for (auto it = this->begin(); it != this->end(); it++) {
        t += it->second;
    }
    return t;
}

double ResourceLedger::MultiplyAndGetSum(ResourceLedger &other) {
    double sum = 0;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        sum += iterator->second * other[iterator->first];
    }
    return sum;
}

ResourceLedger ResourceLedger::SafeDivision(const ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = *this;
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (iterator->second == 0) {
            ledger[iterator->first] = std::numeric_limits<double>::infinity();
        } else if (ledger[iterator->first] == 0) {
            ledger[iterator->first] = 0;
        } else {
            ledger[iterator->first] =
                ledger[iterator->first] / iterator->second;
        }
    }
    return ledger;
}
/// <summary>
/// Finds the smallest value in the Ledger.
/// </summary>
/// <returns>The smallest value in the ledger</returns>
double ResourceLedger::Min() {
    double Minimum = this->begin()->second;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++)
        if (iterator->second < Minimum) Minimum = iterator->second;
    return Minimum;
}

/// <summary>
/// Finds the largest value in the Ledger.
/// </summary>
/// <returns>The largest value in the ledger</returns>
double ResourceLedger::Max() {
    double Maximum = this->begin()->second;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++)
        if (iterator->second > Maximum) Maximum = iterator->second;
    return Maximum;
}




double ResourceLedger::Average() {
    return this->GetSum() / this->size();
}

std::string ResourceLedger::to_string() {
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

ResourceLedger RecipeOutput::operator*(
    const double value) const {
    ResourceLedger ledger;
    ledger[entity] = value * amount;
    return ledger;
}
ResourceLedger RecipeOutput::operator*(ResourceLedger & ledger) const {
    ResourceLedger ret;
    ret[entity] = ledger[entity] * amount;
    return ret;
}

/// <summary>
/// Creates a new resource ledger using the keys from one resource ledger, and the values from annother
/// </summary>
ResourceLedger CopyVals(const ResourceLedger &keys,
                        const ResourceLedger &values) {
    ResourceLedger tkeys = keys;
    for (auto iterator = keys.begin(); iterator != keys.end();
         iterator++) {
        tkeys[iterator->first] = values[iterator->first];
    }
    return tkeys;
}

}  // namespace cqsp::common::components
