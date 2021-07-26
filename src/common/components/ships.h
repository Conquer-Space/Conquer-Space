// Copyright 2021 Conquer Space
#pragma once

#include <entt/entt.hpp>
//#include <vector>

#include "common/components/units.h"

namespace conquerspace {
namespace common {
namespace components {
namespace ships {
struct Ship {
    /*
     * Radius of the body
     */

    types::astronomical_unit radius;

    entt::entity star_system;
};
struct Command {
    /*
     * Radius of the body
     */

    types::astronomical_unit radius;

    entt::entity target;
};
}  // namespace ships
}  // namespace components
}  // namespace common
}  // namespace conquerspace
