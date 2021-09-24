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
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include <vector>
#include <memory>
#include <string>

#include "common/components/area.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/profiler.h"
#include "common/systems/movement/sysmovement.h"
#include "common/systems/economy/syseconomy.h"
#include "common/systems/syspopulation.h"

#include "common/components/event.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/ships.h"
#include "common/components/coordinates.h"
#include "common/components/units.h"

using cqsp::common::systems::simulation::Simulation;
using cqsp::common::Universe;
Simulation::Simulation(Universe &_universe, scripting::ScriptInterface &script_interface) :
    m_universe(_universe), script_runner(_universe, script_interface) {
    namespace cqspcs = cqsp::common::systems;
    AddSystem<cqspcs::SysPopulationGrowth>();
    AddSystem<cqspcs::SysPopulationConsumption>();
    AddSystem<cqspcs::SysFactory>();
    //AddSystem<cqspcs::SysOrbit>();
    AddSystem<cqspcs::SysPath>();
}

void cqsp::common::systems::simulation::Simulation::tick() {
    m_universe.DisableTick();
    m_universe.date.IncrementDate();
    // Get previous tick spacing
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    auto start = std::chrono::high_resolution_clock::now();
    BEGIN_TIMED_BLOCK(Game_Loop);
    BEGIN_TIMED_BLOCK(ScriptEngine);
    this->script_runner.ScriptEngine();
    END_TIMED_BLOCK(ScriptEngine);

    for (auto& sys : system_list) {
        if (m_universe.date.GetDate() % sys->Interval() == 0) {
            sys->DoSystem(m_universe);
        }
    }
    END_TIMED_BLOCK(Game_Loop);
    auto end = std::chrono::high_resolution_clock::now();
    int len = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    int expected_len = 250;
    if (len > expected_len) {
        SPDLOG_WARN("Tick has taken more than {} ms at {} ms", expected_len, len);
    }
}
