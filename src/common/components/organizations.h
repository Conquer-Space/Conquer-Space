/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <entt/entt.hpp>

namespace conquerspace {
namespace components {
struct Organization {};

struct Civilization {
    entt::entity starting_planet;
};
}  // namespace components
}  // namespace conquerspace
