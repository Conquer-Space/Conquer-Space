/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "common/systems/isimulationsystem.h"


namespace conquerspace {
namespace common {
namespace systems {
class SysResourceGen : public ISimulationSystem {
 public:
    void DoSystem(components::Universe& universe);
};

class SysFactoryResourceProduction : public ISimulationSystem {
 public:
    void DoSystem(components::Universe& universe);
};

class SysFactoryResourceConsumption : public ISimulationSystem {
 public:
    void DoSystem(components::Universe& universe);
};
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
