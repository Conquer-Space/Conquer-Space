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
namespace components {
struct Good {
    conquerspace::components::types::meter_cube volume;
    conquerspace::components::types::kilogram mass;
};

struct ResourceLedger : public std::map<entt::entity, double> {
    ResourceLedger operator-(ResourceLedger&);
    ResourceLedger operator+(ResourceLedger&);
    ResourceLedger operator*(double value);
    void operator-=(const ResourceLedger&);
    void operator+=(const ResourceLedger&);
    void operator*=(const double value);

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

struct ResourceGenerator : public ResourceLedger {
};

struct ResourceConverter {
    entt::entity recipe;
};

struct ResourceStockpile : public ResourceLedger {
};
}  // namespace components
}  // namespace conquerspace
