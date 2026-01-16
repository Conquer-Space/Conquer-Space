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

#include "core/systems/economy/economyconfig.h"
#include "core/systems/isimulationsystem.h"

namespace cqsp::core::systems {
/**
 * Resets the market for this tick's market loop.
 * Ideally we do not have this system at all, because memory clears are very inefficient
 * So we will need to find a way to more dynamically reset resource ledgers on the fly.
 */
class SysMarketReset : public ISimulationSystem {
 public:
    explicit SysMarketReset(Game& game) : ISimulationSystem(game) {}
    void DoSystem() override;
    int Interval() override { return ECONOMIC_TICK; }
};
}  // namespace cqsp::core::systems
