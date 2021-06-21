/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <map>
#include <string>

#include <entt/entt.hpp>

#include "common/stardate.h"

namespace conquerspace {
namespace components {
class Universe : public entt::registry {
 public:
    StarDate date;

    std::map<std::string, entt::entity> goods;
    std::map<std::string, entt::entity> recipes;

    void EnableTick() { to_tick = true; }
    void DisableTick() { to_tick = false; }
    bool ToTick() { return to_tick; }

 private:
    bool to_tick = false;
};
}  // namespace components
}  // namespace conquerspace
