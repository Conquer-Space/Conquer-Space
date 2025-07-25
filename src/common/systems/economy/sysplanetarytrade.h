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

#include "common/systems/isimulationsystem.h"
#include "common/universe.h"

namespace cqsp::common::systems {
// System for mines to reduce production so that production will stay stable if the price
// dips too low
// Main goal is to maintain stable pricing
class SysPlanetaryTrade : public ISimulationSystem {
 public:
    explicit SysPlanetaryTrade(Game& game) : ISimulationSystem(game), initial_tick(true) {}
    void DoSystem() override;
    int Interval() override { return components::StarDate::DAY; }

 private:
    bool initial_tick;
};
}  // namespace cqsp::common::systems
