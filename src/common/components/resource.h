/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <entt/entt.hpp>

#include "common/components/units.h"

namespace conquerspace {
namespace common {
namespace components {
struct Good {
    conquerspace::common::components::types::meter_cube volume;
    conquerspace::common::components::types::kilogram mass;
};

struct ResourceLedger : public std::map<entt::entity, double> {
    ResourceLedger operator-(ResourceLedger&);
    ResourceLedger operator+(ResourceLedger&);
    ResourceLedger operator*(double value);
    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const double value);
    bool operator<(const ResourceLedger&);
    bool operator>(const ResourceLedger&);

    void AssignFrom(const ResourceLedger&);

    bool HasGood(entt::entity good) {
        return (*this).find(good) == (*this).end();
    }
};

struct Recipe {
    ResourceLedger input;
    ResourceLedger output;

    float interval;
};

struct FactoryProductivity {
    // Amount generated per generation
    float productivity;
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
}  // namespace components
}  // namespace common
}  // namespace conquerspace
