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

#include "common/systems/isimulationsystem.h"

namespace cqsp::common::systems {
class SysWalletReset : public ISimulationSystem {
 public:
    explicit SysWalletReset(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysEmploymentHandler : public ISimulationSystem {
 public:
    explicit SysEmploymentHandler(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

/// <summary>
/// Generates resources and puts it in the stockpile
/// </summary>
class SysResourceGenerator : public ISimulationSystem {
 public:
    explicit SysResourceGenerator(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

/// <summary>
/// Generates resources for factories for the recipe
/// </summary>
class SysProduction : public ISimulationSystem {
 public:
    explicit SysProduction(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysDemandCreator : public ISimulationSystem {
 public:
    explicit SysDemandCreator(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysFactoryDemandCreator : public ISimulationSystem {
 public:
    explicit SysFactoryDemandCreator(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysGoodSeller : public ISimulationSystem {
 public:
    explicit SysGoodSeller(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysDemandResolver : public ISimulationSystem {
 public:
    explicit SysDemandResolver(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysMarketResolver : public ISimulationSystem {
 public:
    explicit SysMarketResolver(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};
class SysProductionStarter : public ISimulationSystem {
 public:
    explicit SysProductionStarter(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysPriceDetermine : public ISimulationSystem {
 public:
    explicit SysPriceDetermine(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysConsumptionConsume : public ISimulationSystem {
 public:
    explicit SysConsumptionConsume(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};

class SysCommercialProcess : public ISimulationSystem {
 public:
    explicit SysCommercialProcess(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
};
}  // namespace cqsp::common::systems
