/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
class ISimulationSystem {
 public:
    virtual void DoSystem(components::Universe& universe) = 0;
    virtual int Interval() { return 25; }
};
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
