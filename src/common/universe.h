/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include<entt/entt.hpp>

#include "common/stardate.h"

namespace conquerspace {
namespace components {
class Universe {
 public:
    StarDate date;
    entt::registry registry;
};
}  // namespace components
}  // namespace conquerspace
