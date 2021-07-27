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
