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

    bool operator==(const ResourceLedger&);

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

    void AssignFrom(const ResourceLedger&);

    bool HasGood(entt::entity good) {
        return (*this).find(good) != (*this).end();
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

struct ResourceDemand : public ResourceLedger {};
}  // namespace components
}  // namespace common
}  // namespace conquerspace
