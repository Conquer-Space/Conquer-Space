/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>

#include <entt/entt.hpp>

namespace conquerspace {
namespace components {
struct Surface {
    int64_t seed;
};

/*
* If people are living in an area or not
*/
struct Habitation {
    std::vector<entt::entity> settlements;
};

/*
 * People who live there.
*/
struct Settlement {
    std::vector<entt::entity> population;
};
}  // namespace components
}  // namespace conquerspace
