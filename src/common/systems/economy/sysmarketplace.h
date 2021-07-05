/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "common/systems/isimulationsystem.h"


namespace conquerspace {
namespace common {
namespace systems {
class SysMarketSeller : public ISimulationSystem {
 public:
     void DoSystem(components::Universe& universe);
};
}
}
}