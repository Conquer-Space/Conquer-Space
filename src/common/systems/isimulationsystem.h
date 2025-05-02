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

#include <entt/entt.hpp>

#include "common/game.h"
#include "common/universe.h"

namespace cqsp::common::systems {
class ISimulationSystem {
 public:
    explicit ISimulationSystem(Game& game) : game(game) {}
    virtual ~ISimulationSystem() = default;
    virtual void DoSystem() = 0;
    virtual void Init() {};

    /// How often is `DoSystem` is run. It will be run every
    /// n number of ticks when the tick becomes a multiple of the number below.
    /// The default is 24
    virtual int Interval() { return components::StarDate::DAY; }

 protected:
    Game& GetGame() { return game; }
    Universe& GetUniverse() { return game.GetUniverse(); }

 private:
    Game& game;
};
}  // namespace cqsp::common::systems
