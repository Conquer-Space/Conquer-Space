/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace simulation {

/*
 * Main simulation of game.
 */
class Simulation {
 public:
    explicit Simulation(conquerspace::common::components::Universe &_universe)
        : m_universe(_universe) {}
    void tick();

    conquerspace::common::components::Universe &m_universe;
};
}  // namespace simulation
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
