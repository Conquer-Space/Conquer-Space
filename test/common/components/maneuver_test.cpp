/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include "common/game.h"
#include "common/simulation.cpp"
#include "common/systems/movement/sysorbit.h"
#include "engine/asset/packageindex.h"

class ManeuverTestSimulation : public cqsp::common::systems::simulation::Simulation {
 public:
    explicit ManeuverTestSimulation(cqsp::common::Game &game) : cqsp::common::systems::simulation::Simulation(game) {}

    void CreateSystems() override { AddSystem<cqsp::common::systems::SysOrbit>(); }
};

TEST(ManeuverTest, MatchPlaneTest) {
    // Now generate a bunch of orbits and universe and figure out if we can match to them
    // Generate a sim
    cqsp::common::Game game;
    // Initialize a few planets
    // Load the core package
    // Load a folder
    cqsp::asset::PackageIndex index();
    ManeuverTestSimulation simulation(game);
}