/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

namespace cqsp {
namespace common {
namespace systems {
namespace simulation {

/*
 * Main simulation of game.
 */
class Simulation {
 public:
    explicit
    Simulation(cqsp::common::Game &game);

    void tick();

    template <class T>
    void AddSystem() {
        static_assert(std::is_base_of<cqsp::common::systems::ISimulationSystem, T>::value);
        system_list.push_back(std::make_unique<T>(m_game));
    }

 private:
    cqsp::common::Game &m_game;
    std::vector<std::unique_ptr<cqsp::common::systems::ISimulationSystem>> system_list;
    cqsp::common::Universe &m_universe;
};
}  // namespace simulation
}  // namespace systems
}  // namespace common
}  // namespace cqsp
