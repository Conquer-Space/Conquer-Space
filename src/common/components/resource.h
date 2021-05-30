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
};

struct ResourceGenerator {
    std::map<entt::entity, int> output;
};

struct ResourceConverter {
    entt::entity recipe;
    int productivity = 1;
};

struct ResourceStockpile {
    std::map<entt::entity, int> stored;
};
}  // namespace components
}  // namespace conquerspace
