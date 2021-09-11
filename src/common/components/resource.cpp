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

template<class Map, class Function>
Map merge_apply(const Map &m1, const Map &m2, typename Map::mapped_type identity, Function func) {
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    auto comp = m1.value_comp();
    Map res;
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

template<class Map, class Function>
bool MergeCompare(const Map &m1, const Map &m2,  typename Map::mapped_type identity, Function func) {
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

using cqsp::common::components::ResourceLedger;

ResourceLedger ResourceLedger::operator-(const ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = *this;
    ledger -= other;
    return ledger;
}

ResourceLedger ResourceLedger::operator+(const ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = *this;
    ledger += other;
    return ledger;
}

ResourceLedger ResourceLedger::operator*(double value) {
    ResourceLedger ledger;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        ledger[iterator->first] = iterator->second * value;
    }
    return ledger;
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

void ResourceLedger::operator*=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] = iterator->second * value;
    }
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
        (*this)[iterator->first] = iterator->second * value;
    }
}
