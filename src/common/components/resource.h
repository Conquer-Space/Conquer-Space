/*
 * Copyright 2021 Conquer Space
 */
#pragma once
#include <string>
#include <vector>
#include <entt/entt.hpp>

#include "common/components/units.h"

namespace conquerspace {
namespace components {
struct Good {
    conquerspace::components::types::meter_cube volume;
    conquerspace::components::types::kilogram mass;
};

struct ResourceGenerator {
    std::vector<entt::entity> output;
};

struct ResourceConverter {
    std::vector<entt::entity> input;
    std::vector<entt::entity> output;
};
}  // namespace components
}  // namespace conquerspace
