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
class SysMarket : public ISimulationSystem {
 public:
    explicit SysMarket(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
    int Interval() override { return 24; }

    /// <summary>
    /// To be called before the game starts
    /// </summary>
    /// <param name="game"></param>
    static void InitializeMarket(Game& game);
};
}  // namespace cqsp::common::systems
