/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>

namespace conquerspace {
namespace common {
namespace components {
struct Industry {
    std::vector<entt::entity> industries;
};

struct Production {
    float amount;
};

struct Factory {};

struct Mine {};

struct Farm {};
}  // namespace components
}  // namespace common
}  // namespace conquerspace
