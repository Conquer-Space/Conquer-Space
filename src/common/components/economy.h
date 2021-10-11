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
/// <summary>
/// Market that records the resources
/// </summary>
struct Market {
    std::set<entt::entity> participants;
    ResourceLedger prices;
    ResourceLedger demand;
    ResourceLedger supply;

    void AddParticipant(entt::entity participant) {
        participants.insert(participant);
    }
};

/// <summary>
/// Price of a good.
/// This is temporary, because this is to determine initial prices for goods. In the future, good prices
/// will be determined by what the market is willing to bear, and we will not need this anymore.
/// </summary>
struct Price {
    double price;

    operator double() { return price; }
};

/// <summary>
///  Will be handled in the future, but for now is just a market
///
///  Ideas to implement in the future:
///     - Inflation
///     - Fiat or hard
///     - Digital or physical
///     - Pegs
///     - Conversion rates to other currencies
/// </summary>
struct Currency {};

/// <summary>
///  Records the prices of goods and other things
/// </summary>
struct CostTable : public ResourceLedger {};

// TODO(EhWhoAmI): Add multiple currency support
struct Wallet {
    Wallet() = default;
    Wallet(entt::entity _currency, double _balance) : balance(_balance), currency(_currency) {}

    Wallet& operator+=(double amount) {
        this->balance += amount;
        change += amount;
        return *this;
    }
    Wallet& operator-=(double amount) {
        this->balance -= amount;
        change -= amount;
        GDP_change += amount;
        // Record the money delta since last reset
        return *this;
    }

    operator double() { return balance; }

    Wallet& operator=(double _balance) {
        change += (_balance - balance);
        if ((_balance - balance) < 0) {
            GDP_change += _balance - balance;
        }
        balance = _balance;
        return *this;
    }

    double GetBalance() { return balance; }
    void Reset() {
        change = 0;
        GDP_change = 0;
    }
    double GetGDPChange() { return GDP_change; }
 private:
    double balance = 0;
    double change = 0;
    // Only records when spending money, so when money decreases
    double GDP_change = 0;
    entt::entity currency;
};

/// <summary>
/// An actor in a market that trades goods.
/// </summary>
struct MarketAgent {
    entt::entity market;
};

/// <summary>
/// An entity where the market is based, and the resources are traded.
/// </summary>
struct MarketCenter {
    entt::entity market;
};

/// <summary>
/// Represents commercial areas and other amenities that generate economic activity. They don't export goods
/// but they play an intergral role in tax revenue, and population consumption, and maybe tourism in the future.
/// </summary>
struct Commercial {
    /// The city that it's based in
    entt::entity city;
    int size;
};

// Something that hires people, and will pay the people
struct Employer {
    int population_needed;
    int population_fufilled;
    entt::entity segment;
};

/// <summary>
/// Population segment that is employed
/// </summary>
struct Employee {
    /// <summary>
    /// The population that is available to work
    /// </summary>
    int working_population;
    /// <summary>
    /// The current population is currently working.
    /// </summary>
    int employed_population;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
