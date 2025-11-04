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
#pragma once

#include <memory>
#include <vector>

#include "common/game.h"
#include "common/systems/isimulationsystem.h"

namespace cqsp::common::systems::simulation {
/// <summary>
/// Main simulation of game.
///</summary>
/// To add a simulation in the game, add a class extending from `cqsp::common::systems::ISimulationSystem`
/// in the constructor.
/// ```
/// AddSystem<SimSystemName>();
/// ```
///
class Simulation {
 public:
    explicit Simulation(Game &game);

    /// <summary>
    /// 1 game tick, runs every single system that is added.
    /// </summary>
    void tick();
    void Init();

    template <class T>
    void AddSystem() {
        static_assert(std::is_base_of<ISimulationSystem, T>::value);
        system_list.push_back(std::make_unique<T>(m_game));
    }

 protected:
    virtual void CreateSystems();

 private:
    Game &m_game;
    /// <summary>
    /// Holds all the systems.
    /// </summary>
    std::vector<std::unique_ptr<ISimulationSystem>> system_list;
    Universe &m_universe;
};
}  // namespace cqsp::common::systems::simulation
