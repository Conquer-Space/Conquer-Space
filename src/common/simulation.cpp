/*
* Copyright 2021 Conquer Space
*/
#include "common/simulation.h"

#include "common/components/resource.h"

void conquerspace::systems::simulation::Simulation::tick() {
    // Tick date
    m_universe.date.IncrementDate();
}
