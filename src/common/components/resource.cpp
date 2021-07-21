/*
 * Copyright 2021 Conquer Space
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

using conquerspace::common::components::ResourceLedger;

ResourceLedger ResourceLedger::operator-(ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = merge_apply(*this, other, 0, [](double a, double b) { return a - b; });
    return ledger;
}

ResourceLedger ResourceLedger::operator+(ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = merge_apply(*this, other, 0, [](double a, double b) { return a + b; });
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

bool ResourceLedger::operator>(const ResourceLedger &ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a > b; });
}

bool ResourceLedger::operator<=(
    const ResourceLedger & ledger) {
    return MergeCompare(*this, ledger, 0, [](double a, double b) { return a <= b; });
}

template<class Map, class Function>
bool CompareMapDouble(const Map &m1, typename Map::mapped_type compare_to, Function func) {
    bool op = true;
    if (m1.size() == 0) {
        return func(0, compare_to);
    }
    for (auto iterator = m1.begin(); iterator != m1.end(); iterator++) {
        op &= func(iterator->second, compare_to);
    }
    return op;
}

bool ResourceLedger::operator>(const double &i) {
    return CompareMapDouble(*this, i, [](double a, double b) { return a > b; });
}

bool ResourceLedger::operator<(const double & i) {
    return CompareMapDouble(*this, i, [](double a, double b) { return a < b; });
}

bool ResourceLedger::operator==(const double &i) {
    return CompareMapDouble(*this, i, [](double a, double b) { return a == b; });
}

bool ResourceLedger::operator<=(const double &i) {
    return CompareMapDouble(*this, i, [](double a, double b) { return a <= b; });
}

bool ResourceLedger::operator>=(const double &i) {
    return CompareMapDouble(*this, i, [](double a, double b) { return a >= b; });
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

void ResourceLedger::AssignFrom(const ResourceLedger &ledger) {
    for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
        (*this)[iterator->first] = iterator->second;
    }
}
