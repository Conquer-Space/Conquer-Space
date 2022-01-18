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

template<class Map, class Function>
bool MergeCompare(const Map &m1, const Map &m2,  typename Map::mapped_type identity, Function func) {
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    auto comp = m1.value_comp();
    bool op = true;
    while (true) {
        bool end1 = it1 == m1.end();
        bool end2 = it2 == m2.end();
        if (end1 && end2) {
            break;
        }

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

using cqsp::common::components::ResourceLedger;


#ifdef TRACY_ENABLE
int ResourceLedger::stockpile_additions = 0;
#define STOCKPILE_ADDITION ++ResourceLedger::stockpile_additions;
#else
#define STOCKPILE_ADDITION
#endif  // TRACY_ENABLE

bool cqsp::common::components::ResourceLedger::EnoughToTransfer(const ResourceLedger &amount) {
    bool b = true;
    for (auto it : amount) {
        b &= (*this)[it.first] >= it.second;
    }
    return b;
}

ResourceLedger ResourceLedger::operator-(const ResourceLedger &other) const {
    ResourceLedger ledger;
    ledger = *this;
    ledger -= other;
    return ledger;
}

ResourceLedger ResourceLedger::operator+(const ResourceLedger &other) const {
    ResourceLedger ledger;
    ledger = *this;
    ledger += other;
    return ledger;
}

ResourceLedger ResourceLedger::operator*(double value) const {
    ResourceLedger ledger;
    for (auto it : (*this)) {
        ledger[it.first] = it.second * value;
    }
    return ledger;
}

ResourceLedger cqsp::common::components::ResourceLedger::operator*(ResourceLedger &other) const {
    ResourceLedger ledger;
    ledger = *this;
    ledger *= other;
    return ledger;
}

void ResourceLedger::operator-=(const ResourceLedger &other) {
    for (auto it : other) {
        (*this)[it.first] -= it.second;
    }
}

void ResourceLedger::operator+=(const ResourceLedger &other) {
    for (auto iterator : other) {
        (*this)[iterator.first] += iterator.second;
    }
}

void ResourceLedger::operator*=(const double value) {
    for (auto iterator : (*this)) {
        (*this)[iterator.first] = iterator.second * value;
    }
}

void cqsp::common::components::ResourceLedger::operator*=(ResourceLedger &other) {
    for (auto iterator : (*this)) {
        (*this)[iterator.first] = iterator.second * other[iterator.first];
    }
}

// Not sure if this is faster than a function, but wanted to have fun with the preprocessor,
// so here we go
#define compare(map, compare_to, comparison) \
    bool op = true; \
    if ((map).empty()) { \
        return 0 comparison compare_to; \
    } \
    for (auto iterator : (map)) { \
        op &= iterator.second comparison compare_to; \
    } \
    return op;

bool ResourceLedger::operator>(const double &i) const {
    compare(*this, i, >)
}

bool ResourceLedger::operator<(const double & i) const {
    compare(*this, i, <)
}

bool ResourceLedger::operator==(const double &i) const {
    compare(*this, i, ==)
}

bool ResourceLedger::operator<=(const double &i) const {
    compare(*this, i, <=)
}

bool ResourceLedger::operator>=(const double &i) const {
    compare(*this, i, >=)
}

bool ResourceLedger::operator>=(const ResourceLedger &ledger) const {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a >= b; });
}

bool ResourceLedger::operator==(const ResourceLedger &ledger) const {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a == b; });
}

bool ResourceLedger::operator<(const ResourceLedger &ledger) const {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a < b; });
}

bool ResourceLedger::operator>(const ResourceLedger &ledger) const {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a > b; });
}

bool ResourceLedger::operator<=(const ResourceLedger &ledger) const {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a <= b; });
}

void ResourceLedger::AssignFrom(const ResourceLedger &ledger) {
    for (auto iterator : ledger) {
        (*this)[iterator.first] = iterator.second;
    }
}

void ResourceLedger::TransferTo(ResourceLedger& ledger_to, const ResourceLedger &amount) {
    for (auto iterator : amount) {
        (*this)[iterator.first] -= iterator.second;
        ledger_to[iterator.first] += iterator.second;
    }
}

void ResourceLedger::MultiplyAdd(const ResourceLedger & other, double value) {
    STOCKPILE_ADDITION;
    for (auto iterator : other) {
        (*this)[iterator.first] += iterator.second * value;
    }
}

void ResourceLedger::RemoveResourcesLimited(const ResourceLedger & other) {
    for (auto iterator : other) {
        double &t = (*this)[iterator.first];
         t -= iterator.second;
         if (t < 0) {
             t = 0;
         }
    }
}

ResourceLedger ResourceLedger::LimitedRemoveResources(const ResourceLedger& other) {
    ResourceLedger removed;
    for (auto iterator : other) {
        double &t = (*this)[iterator.first];
        if (t > iterator. second) {
            removed[iterator.first] = iterator.second;
            t -= iterator.second;
        } else {
            removed[iterator.first] = t;
            t = 0;
        }
    }
    return removed;
}

double ResourceLedger::GetSum() {
    double t = 0;
    for (auto it : (*this)) {
        t += it.second;
    }
    return t;
}

double ResourceLedger::MultiplyAndGetSum(ResourceLedger &ledger) {
    double sum = 0;
    for (auto iterator : (*this)) {
        sum += iterator.second * ledger[iterator.first];
    }
    return sum;
}

std::string ResourceLedger::to_string() {
    std::string str = "{";
    for (auto it : (*this)) {
        str.append(" ");
        str.append(std::to_string(static_cast<std::uint32_t>(it.first)));
        str.append(",");
        str.append(std::to_string(it.second));
    }
    str.append("}");
    return str;
}
