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
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

#include "common/components/area.h"
#include "common/components/coordinates.h"
#include "common/components/event.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/units.h"
#include "common/systems/economy/sysagent.h"
#include "common/systems/economy/sysfactory.h"
#include "common/systems/economy/sysfinance.h"
#include "common/systems/economy/sysinfrastructure.h"
#include "common/systems/economy/sysmarket.h"
#include "common/systems/economy/sysmarketreset.h"
#include "common/systems/economy/sysplanetarytrade.h"
#include "common/systems/economy/syspopulation.h"
#include "common/systems/history/sysmarkethistory.h"
#include "common/systems/movement/sysmovement.h"
#include "common/systems/navy/sysnavy.h"
#include "common/systems/science/syssciencelab.h"
#include "common/systems/science/systechnology.h"
#include "common/systems/scriptrunner.h"
#include "common/util/profiler.h"

using cqsp::common::Universe;
using cqsp::common::systems::simulation::Simulation;

Simulation::Simulation(cqsp::common::Game& game) : m_game(game), m_universe(game.GetUniverse()) {
    namespace cqspcs = cqsp::common::systems;
    AddSystem<cqspcs::SysScript>();
    AddSystem<cqspcs::SysWalletReset>();

    AddSystem<cqspcs::SysScienceLab>();
    AddSystem<cqspcs::SysTechProgress>();

    // Economy progress
    AddSystem<cqspcs::SysMarketReset>();

    AddSystem<cqspcs::InfrastructureSim>();
    AddSystem<cqspcs::SysPopulationConsumption>();
    AddSystem<cqspcs::SysProduction>();

    // AddSystem<cqspcs::SysAgent>();
    AddSystem<cqspcs::SysMarket>();
    AddSystem<cqspcs::SysPlanetaryTrade>();
    AddSystem<cqspcs::history::SysMarketHistory>();

    // Movement
    AddSystem<cqspcs::SysOrbit>();
}

void Simulation::Init() {
    namespace cqspcs = cqsp::common::systems;
    cqspcs::SysMarket::InitializeMarket(m_game);

    for (auto& sys : system_list) {
        sys->Init();
    }
}

void Simulation::tick() {
    m_universe.DisableTick();
    m_universe.date.IncrementDate();
    // Get previous tick spacing
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
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
