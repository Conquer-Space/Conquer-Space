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

#include <map>
#include <queue>

#include <entt/entt.hpp>

#include "core/components/market.h"
#include "core/components/resource.h"

namespace cqsp::core::components::infrastructure {
struct TransportedGood {
    entt::entity good;
    double amount;
    double fulfilled;
    double priority;

    TransportedGood() : good(entt::null), amount(0.0), fulfilled(0.0), priority(0) {}
    explicit TransportedGood(const MarketOrder& order, entt::entity _good)
        : good(_good), amount(order.amount), fulfilled(0.0), priority(0) {}
};

/**
 * for now, we'll only support vertical rocket based launches
 * I think the future might be just space planes that can take off from airports or other stuff
 * so that will unlock a lot of possibilities to go to space
 * however idk how that will have an implication for the game as every single air port will become a spaceport
 * which means that we could launch like hundreds of rockets a day, which is kind of nuts
 * But for now, we'll support a limited amount of space launch systems
 */
struct SpacePort {
    // The key is target, and queue
    // The entt entity must have an orbit, and we must be able to rendezvous to that entity
    // So this is the list of goods that it wants to deliver to different places
    std::map<entt::entity, std::vector<TransportedGood>> deliveries;

    double launch_cadence = 0;
    int launchpads = 0;
    entt::entity reference_body = entt::null;

    ResourceLedger demanded_resources;
    ResourceLedger demanded_resources_rate;
    ResourceLedger output_resources;
    ResourceLedger output_resources_rate;
    ResourceLedger resource_stockpile;
};
}  // namespace cqsp::core::components::infrastructure
