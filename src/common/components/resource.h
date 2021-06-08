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

struct Recipe {
    std::map<entt::entity, int> input;
    std::map<entt::entity, int> output;

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

struct ResourceGenerator {
    std::map<entt::entity, int> output;
};

struct ResourceConverter {
    entt::entity recipe;
};

struct ResourceStockpile {
    std::map<entt::entity, int> stored;
};

class ResourceLedger {
};
}  // namespace components
}  // namespace conquerspace
