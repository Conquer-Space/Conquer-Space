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
#include "common/components/economy.h"

#include <limits>

using cqsp::common::components::Market;
using cqsp::common::components::ResourceStockpile;

void Market::AddSupply(const ResourceLedger& stockpile) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].supply += stockpile_element.second;
    }
}

void Market::AddSupply(const ResourceLedger& stockpile, double multiplier) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].supply += stockpile_element.second * multiplier;
    }
}

void Market::AddDemand(const ResourceLedger& stockpile) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].demand += stockpile_element.second;
    }
}

void Market::AddDemand(const ResourceLedger& stockpile, double multiplier) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].demand += stockpile_element.second * multiplier;
    }
}

double Market::GetPrice(const ResourceLedger& stockpile) {
    double price = 0;
    for (const auto& element : stockpile) {
        price += market_information[element.first].price * element.second;
    }
    return price;
}

double Market::GetSDRatio(const entt::entity& good) { return market_information[good].sd_ratio(); }

double Market::GetSupply(const entt::entity& good) { return market_information[good].supply; }

double Market::GetDemand(const entt::entity& good) { return market_information[good].demand; }

double Market::GetAndMultiplyPrice(const ResourceLedger& ledger) {
    double price = 0;
    for (const auto& led : ledger) {
        price += market_information[led.first].price * led.second;
    }
    return price;
}

double Market::GetAndMultiplyPrice(const RecipeOutput& ledger) {
    return ledger.amount * market_information[ledger.entity].price;
}

void Market::DividePriceLedger(ResourceLedger& ledger) {
    for (auto& val : ledger) {
        val.second /= market_information[val.first].price;
    }
}

double Market::GetLowestSDRatio(ResourceLedger& ledger) {
    double lowest = std::numeric_limits<double>::infinity();
    for (auto& in : ledger) {
        double val = last_market_information[in.first].sd_ratio();
        if (lowest < val) {
            lowest = val;
        }
    }
    return lowest;
}

double Market::GetPrice(const entt::entity& good) { return market_information[good].price; }

double cqsp::common::components::MarketElementInformation::sd_ratio() {
    if (supply == 0) {
        return std::numeric_limits<double>::infinity();
    } else if (demand == 0) {
        return 0;
    } else {
        return supply / demand;
    }
}
