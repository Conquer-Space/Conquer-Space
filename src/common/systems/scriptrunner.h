/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <vector>

#include "common/scripting/scripting.h"
#include "common/systems/isimulationsystem.h"
#include "common/universe.h"

namespace cqsp::common::systems {
/// <summary>
/// Runs scripts that are added to the game
/// </summary>
///
/// To run a script every single tick, you have to do this:
/// ```lua
/// local test_event = {
///     -- you can define all sorts of variables needed here
/// }
///
/// local test_event:on_tick()
///     -- All sorts of events take place here
/// end
///
/// -- then add it to the event queue.
/// events:insert(test_event)
/// ```
///
/// This is likely to be unoptimized because it runs all the scripts every
/// single tick.
class SysScript : public cqsp::common::systems::ISimulationSystem {
 public:
    explicit SysScript(Game& game);
    ~SysScript();
    void DoSystem();
    int Interval() { return 1; }

 private:
    std::vector<sol::table> events;
};
} // namespace cqsp::common::systems
