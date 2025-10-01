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
    auto planetary_markets = GetUniverse().view<components::Market, components::infrastructure::SpacePort>();
    // Let's make a directed graph and try to get the time to transfer to each thing
    // Alright so we should do path finding
    for (entt::entity entity : planetary_markets) {
        // Now let's make a table for the resources that we need
        // Get the children entites and check if they have a space port
        auto& market_component = GetUniverse().get<components::Market>(entity);
        // So now we want to figure out which goods have a chronic shortage that we can't fulfill, and are cheap enough
        // that we can ship with some margin of profit
        auto& spaceport_component = GetUniverse().get<components::infrastructure::SpacePort>(entity);
        // Get all the values that are lacking
        for (auto& [good, shortage] : market_component.chronic_shortages) {
            if (shortage > 0) {
                // Then we should probably add this to the spaceport queue
                // Now we also need a way to account for whatever's already in the queue so we don't queue multiple requests
                // then add it to the list...
                // Since we won't get resources fulfilled
                // See if the thing
                // Or something like that
                double expected_input = market_component.demand()[good] * shortage;
                market_component.resource_fulfilled[good] = shortage;
            }
        }
    }

    // Now get the spaceports and see if we can fulfill the requirements, and if we can in the attached market, let's
    // ship it out
    auto space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
}
}  // namespace cqsp::common::systems
