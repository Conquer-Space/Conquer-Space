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
#include "common/systems/economy/sysinterplanetarytrade.h"

#include "common/components/economy.h"
#include "common/components/spaceport.h"

namespace cqsp::common::systems {
void SysInterplanetaryTrade::DoSystem() {
    // Connect all the space ports together and then do stuff
    // maybe we have to make a graph or something to be able to compute it
    // Since transport ships are descrete we need a way to organize them as well
    // There are like 3 ways to schedule
    // One is to have regular trips, like a bus or something like that
    // one is specially scheduled for a specific type of good
    // one is you wait there and wait until it is full and then send
    // Should we support the first two models?
    // then how do we interact with it
    // So do we have a schedule for a space port
    // then say like we have this amount of shipment from it
    // then the spaceport will take from the queue/schedule
    // maybe an aging priority queue? We add a few things?
    // Alright so...
    // We have a bunch of space ports that act as providers and other stuff
    // Alright so we have a bunch of spaceports that want to deliver goods, we should
    // Get the space ports that want to launch something
    auto planetary_markets = GetUniverse().view<components::Market, components::PlanetaryMarket>();
    // Let's make a directed graph and try to get the time to transfer to each thing
    // Alright so we should do path finding
    for (entt::entity entity : planetary_markets) {
        // Now get the difference, and try to organize it
        // do we just order it based off the price, and see if someone wants to deliver to
        // a place
        // But we also have to consider orbital maneuvers
        // So what do we prioritize by time/delta-v?
        // Since going to space is gonna be easy
    }
    // Make a directed graph for the orbits

    auto space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
    for (entt::entity entity : space_ports) {
        // Now see who wants to deliver and who doesn't want to
        // How do we order the priority of which space port delivers to who?
    }
}
}  // namespace cqsp::common::systems
