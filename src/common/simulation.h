/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <memory>
#include <vector>

#include "common/universe.h"
#include "common/systems/isimulationsystem.h"

#include "common/systems/scriptrunner.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace simulation {

/*
 * Main simulation of game.
 */
class Simulation {
 public:
    explicit Simulation(conquerspace::common::components::Universe &_universe,
                        scripting::ScriptInterface &script_interface);
    void tick();

    template <class T>
    void AddSystem() {
        system_list.push_back(std::make_unique<T>());
    }

 private:
    conquerspace::common::systems::SysEventScriptRunner script_runner;
    std::vector<std::unique_ptr<conquerspace::common::systems::ISimulationSystem>> system_list;
    conquerspace::common::components::Universe &m_universe;
};
}  // namespace simulation
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
