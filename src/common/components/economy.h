/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <string>
#include <vector>
#include <entt/entt.hpp>

namespace conquerspace {
namespace common {
namespace components {
struct Market {
    std::vector<entt::entity> participants;
};

struct MarketParticipant {
    entt::entity market;
};

struct MarketCenter {
    entt::entity market;
};
}  // namespace components
}  // namespace common
}  // namespace conquerspace
