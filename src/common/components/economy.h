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
#include <set>
#include <map>

#include <entt/entt.hpp>

#include "common/components/resource.h"

namespace cqsp {
namespace common {
namespace components {
struct Market {
    std::set<entt::entity> participants;
    ResourceLedger prices;
    ResourceLedger demand;
    ResourceLedger supply;

    void AddParticipant(entt::entity participant) {
        participants.insert(participant);
    }
};

struct Price {
    double price;

    operator double() { return price; }
};

// Will be handled in the future
struct Currency {};

// Records the prices of goods and other things
struct CostTable : public ResourceLedger {};

// TODO(EhWhoAmI): Add multiple currency support
struct Wallet {
    Wallet() = default;
    Wallet(entt::entity _currency, double _balance) : balance(_balance), currency(_currency) {}

    double balance = 0;
    entt::entity currency;
};

struct MarketAgent {
    entt::entity market;
};

struct MarketCenter {
    entt::entity market;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
