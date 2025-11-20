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
#include "core/simulation.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

#include "core/components/area.h"
#include "core/components/coordinates.h"
#include "core/components/event.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/resource.h"
#include "core/components/ships.h"
#include "core/components/units.h"
#include "core/systems/economy/sysagent.h"
#include "core/systems/economy/sysfinance.h"
#include "core/systems/economy/sysinfrastructure.h"
#include "core/systems/economy/sysinterplanetarytrade.h"
#include "core/systems/economy/sysmarket.h"
#include "core/systems/economy/sysmarketreset.h"
#include "core/systems/economy/sysplanetarytrade.h"
#include "core/systems/economy/syspopulation.h"
#include "core/systems/economy/sysproduction.h"
#include "core/systems/economy/sysspaceport.h"
#include "core/systems/history/sysmarkethistory.h"
#include "core/systems/movement/sysorbit.h"
#include "core/systems/science/syssciencelab.h"
#include "core/systems/science/systechnology.h"
#include "core/systems/scriptrunner.h"
#include "core/util/profiler.h"

namespace cqsp::core::systems::simulation {
Simulation::Simulation(Game& game) : m_game(game), m_universe(game.GetUniverse()) {}

void Simulation::CreateSystems() {
    AddSystem<SysScript>();
    AddSystem<SysWalletReset>();

    AddSystem<SysScienceLab>();
    AddSystem<SysTechProgress>();

    // Economy progress
    AddSystem<SysMarketReset>();
    AddSystem<SysSpacePort>();

    AddSystem<InfrastructureSim>();
    AddSystem<SysPopulationConsumption>();
    AddSystem<SysProduction>();

    /* Since the market ticks after all consumption and production is computed,
     the prices that we compute the profits and consumption are going to be one 
     tick delayed.

     This means that the market might not be as responsive as we'd expect to
     changes in price. */
    AddSystem<SysMarket>();
    AddSystem<SysPlanetaryTrade>();
    AddSystem<SysInterplanetaryTrade>();
    AddSystem<history::SysMarketHistory>();

    // Movement
    AddSystem<SysOrbit>();
}

void Simulation::Init() {
    CreateSystems();
    for (auto& sys : system_list) {
        sys->Init();
    }
}

void Simulation::tick() {
    m_universe.DisableTick();
    m_universe.date.IncrementDate();
    // Get previous tick spacing

    auto start = std::chrono::high_resolution_clock::now();
    BEGIN_TIMED_BLOCK(Game_Loop);

    for (auto& sys : system_list) {
        if (m_universe.date.GetDate() % sys->Interval() == 0) {
            sys->DoSystem();
        }
    }
    END_TIMED_BLOCK(Game_Loop);
    auto end = std::chrono::high_resolution_clock::now();
    int len = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const int expected_len = 250;
    if (len > expected_len) {
        SPDLOG_WARN("Tick has taken more than {} ms at {} ms", expected_len, len);
    }
}
}  // namespace cqsp::core::systems::simulation
