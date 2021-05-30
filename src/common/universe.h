/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include<entt/entt.hpp>
#include <map>

#include "common/stardate.h"

namespace conquerspace {
namespace components {
class Universe {
 public:
    StarDate date;
    entt::registry registry;

    std::map<std::string, entt::entity> goods;
    std::map<std::string, entt::entity> recipes;
};
}  // namespace components
}  // namespace conquerspace
