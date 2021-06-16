#include "common/components/resource.h"

#include <spdlog/spdlog.h>

template<class Map, class Function>
Map merge_apply(const Map &m1, const Map &m2,
                 typename Map::mapped_type identity, Function func) {
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

conquerspace::components::ResourceLedger conquerspace::components::ResourceLedger::operator-(
    ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = merge_apply(*this, other, 0, [](int a, int b) { return a - b; });
    return ledger;
}

conquerspace::components::ResourceLedger conquerspace::components::ResourceLedger::operator+(
    ResourceLedger &other) {
    ResourceLedger ledger;
    ledger = merge_apply(*this, other, 0, [](int a, int b) { return a + b; });
    return ledger;
}

conquerspace::components::ResourceLedger conquerspace::components::ResourceLedger::operator*(
    double value) {
    ResourceLedger ledger;
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        ledger[iterator->first] = iterator->second * value;
    }
    return ledger;
}

void conquerspace::components::ResourceLedger::operator-=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (HasGood(iterator->first)) {
            (*this)[iterator->first] = 0;
        }
        (*this)[iterator->first] -= iterator->second;
    }
}

void conquerspace::components::ResourceLedger::operator+=(const ResourceLedger &other) {
    for (auto iterator = other.begin(); iterator != other.end(); iterator++) {
        if (HasGood(iterator->first)) {
            (*this)[iterator->first] = 0;
        }
        (*this)[iterator->first] += iterator->second;
    }
}

void conquerspace::components::ResourceLedger::operator*=(const double value) {
    for (auto iterator = this->begin(); iterator != this->end(); iterator++) {
        (*this)[iterator->first] = iterator->second * value;
    }
}
