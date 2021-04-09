/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <vector>
#include <entt/entt.hpp>

#include "common/components/units.h"

namespace conquerspace {
namespace components {
namespace bodies {
struct Body {
    /*
    * Radius of the body
    */
    types::kilometer radius;
};

struct Terrain {
    int seed;
};

struct Star { };

/*
* A collection of stuff
*/
struct StarSystem {
    std::vector<entt::entity> bodies;
};
}  // namespace bodies
}  // namespace components
}  // namespace conquerspace
