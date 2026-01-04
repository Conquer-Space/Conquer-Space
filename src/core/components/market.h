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
#include <set>
#include <string>
#include <vector>

#include <entt/entt.hpp>

#include "core/components/resource.h"

namespace cqsp::core::components {
/// <summary>
/// Historical information about the market
/// Might change this to a different type of resource ledger so that we don't have so many lookups
/// </summary>
struct MarketInformation {
 private:
    // I forgot why we have 2 separate ledgers for supply and demand
    ResourceMap _demand;
    ResourceMap _supply;

 public:
    ResourceMap sd_ratio;

    /// <summary>
    /// The amount of goods that changed hands. We can use this to calculate the
    /// GDP
    /// </summary>
    ResourceMap volume;
    ResourceMap price;

    ResourceMap chronic_shortages;

    ResourceMap trade;

    ResourceMap resource_fulfilled;

    ResourceMap production;
    ResourceMap consumption;

    void ResetLedgers() {
        // Reset the ledger values
        demand().clear();
        supply().clear();
    }

    ResourceMap& supply() { return _supply; }
    ResourceMap& demand() { return _demand; }
};

struct MarketElementInformation {
    // Sum of the resources traded last time.
    double supply;
    double demand;
    double price;
    double price_ratio;
    double sd_ratio;
    double inputratio;
};

struct MarketOrder {
    /**
     * If it is a buy order it is the entity that wants to buy, if it is a sell order, it is
     * the entity that is selling it.g
     */
    entt::entity target;
    double amount;
    double price;

    MarketOrder(entt::entity target, double amount, double price) : target(target), amount(amount), price(price) {}
};

// A planetary market must have a regular market as well
struct PlanetaryMarket {
    std::map<entt::entity, std::vector<MarketOrder>> demands;
    std::map<entt::entity, std::vector<MarketOrder>> requests;
    // Resources supplied by the interplanetary market
    ResourceMap supplied_resources;
    ResourceMap supply_difference;
};

struct Market : MarketInformation {
    std::vector<MarketInformation> history;

    std::map<entt::entity, MarketElementInformation> market_information;
    std::map<entt::entity, MarketElementInformation> last_market_information;

    std::set<entt::entity> participants;

    entt::basic_sparse_set<entt::entity> connected_markets;

    ResourceMap market_access;

    entt::entity parent_market = entt::null;

    double GDP = 0;
    // How much money we are creating from thin air
    // Cumulative deficit
    double deficit = 0;
    // Deficit in last tick
    double last_deficit = 0;
    // Math
    void AddSupply(const ResourceMap& stockpile);
    void AddSupply(const ResourceMap& stockpile, double multiplier);
    void AddDemand(const ResourceMap& stockpile);
    void AddDemand(const ResourceMap& stockpile, double multiplier);

    double GetPrice(const ResourceMap& stockpile);
    double GetPrice(const entt::entity& good);
    double GetSDRatio(const entt::entity& good);
    double GetSupply(const entt::entity& good);
    double GetDemand(const entt::entity& good);

    void AddParticipant(entt::entity participant) { participants.insert(participant); }

    MarketElementInformation& operator[](entt::entity ent) { return market_information[ent]; }

    auto begin() { return market_information.begin(); }

    auto end() { return market_information.end(); }
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
struct CostTable : public ResourceMap {};

// TODO(EhWhoAmI): Add multiple currency support
struct Wallet {
    Wallet() = default;
    Wallet(entt::entity _currency, double _balance) : balance(_balance), currency(_currency) {}

    Wallet& operator+=(const double amount) {
        this->balance += amount;
        change += amount;
        return *this;
    }
    Wallet& operator-=(const double amount) {
        this->balance -= amount;
        change -= amount;
        GDP_change += amount;
        // Record the money delta since last reset
        return *this;
    }

    // Basic multiplication that logs the change
    // TODO(EhWhoAmI): Make sure this is correct
    Wallet& operator*=(const double coefficent) {
        double newbalance = this->balance * coefficent;
        double change = newbalance - this->balance;
        if (change > 0) {
            *this += change;
        } else if (change < 0) {
            *this -= change * -1;
        }
        return *this;
    }

    operator double() const { return balance; }

    Wallet& operator=(double _balance) {
        change += (_balance - balance);
        if ((_balance - balance) < 0) {
            GDP_change += _balance - balance;
        }
        balance = _balance;
        return *this;
    }

    double GetBalance() const { return balance; }

    double GetChange() const { return change; }

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

// This trade node has international connections
// For space connections, the spaceport struct exists
struct InternationalPort {};

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
struct LaborInformation {
    /// <summary>
    /// The population that is available to work
    /// </summary>
    int working_population;
    /// <summary>
    /// The current population is currently working.
    /// </summary>
    int employed_population;
};

struct FactoryProducing {};

// This facility is bankrolled by something else, so if they run out of money
// they can go to this wallet to ask for cash?
struct Owned {
    entt::entity owner;
};

struct TradePartners : std::vector<entt::entity> {};
}  // namespace cqsp::core::components
