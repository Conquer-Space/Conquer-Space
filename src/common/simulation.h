/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "common/universe.h"

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
    conquerspace::common::components::Universe &m_universe;
    conquerspace::common::systems::SysEventScriptRunner script_runner;
};
}  // namespace simulation
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
