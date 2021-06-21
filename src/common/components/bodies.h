/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <vector>
#include <entt/entt.hpp>

#include "common/components/units.h"

namespace conquerspace {
namespace common {
namespace components {
namespace bodies {
struct Body {
    /*
     * Radius of the body
     */
    types::astronomical_unit radius;

    entt::entity star_system;
};

struct Terrain {
    int seed;
};

struct Star {};

struct Planet {};

struct LightEmitter {};
/*
 * A collection of stuff
 */
struct StarSystem {
    std::vector<entt::entity> bodies;
};
}  // namespace bodies
}  // namespace components
}  // namespace common
}  // namespace conquerspace
