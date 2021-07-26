/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "common/systems/isimulationsystem.h"


namespace conquerspace {
namespace common {
namespace systems {
class SysOrbit : public ISimulationSystem {
 public:
     void DoSystem(components::Universe& universe);
     int Interval();
};

class SysPath : public ISimulationSystem {
 public:
     void DoSystem(components::Universe& universe);
     int Interval();
};

class SysMove : public ISimulationSystem {
   public:
    void DoSystem(components::Universe& universe);
};
}
}
}