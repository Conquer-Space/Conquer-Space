/*
* Copyright 2021 Conquer Space
*/
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include "common/components/area.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/profiler.h"
#include "common/systems/sysresourcegen.h"
#include "common/systems/economy/sysmarketplace.h"

conquerspace::common::systems::simulation::Simulation::Simulation(
                    conquerspace::common::components::Universe& _universe) : m_universe(_universe) {
    AddSystem<conquerspace::common::systems::SysResourceGen>();
    AddSystem<conquerspace::common::systems::SysFactoryResourceProduction>();
    AddSystem<conquerspace::common::systems::SysMarketSeller>();
    // Buy from system, or something
    AddSystem<conquerspace::common::systems::SysFactoryResourceConsumption>();
}

void conquerspace::common::systems::simulation::Simulation::tick() {
    m_universe.DisableTick();
    // Get previous tick spacing
    namespace cqspc = conquerspace::common::components;

    for (auto& sys : system_list) {
        if (m_universe.date.GetDate() % sys->Interval() == 0) {
            sys->DoSystem(m_universe);
        }
    }
    m_universe.date.IncrementDate();
}
