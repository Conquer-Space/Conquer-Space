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

#include <string>
#include <vector>
#include <entt/entt.hpp>

#include "common/components/resource.h"

namespace cqsp {
namespace common {
namespace components {
struct Market {
    std::set<entt::entity> participants;
    std::map<entt::entity, double> prices;
    ResourceLedger demand;
    ResourceLedger supply;

    void AddParticipant(entt::entity participant) {
        participants.insert(participant);
    }
};

struct MarketParticipant {
    entt::entity market;
};

struct MarketCenter {
    entt::entity market;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
