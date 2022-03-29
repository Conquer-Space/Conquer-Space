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

using cqsp::common::components::Market;
using cqsp::common::components::ResourceStockpile;

void Market::AddSupply(const ResourceLedger& stockpile) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].supply += stockpile_element.second;
    }
}

void cqsp::common::components::Market::AddSupply(const ResourceLedger& stockpile, double multiplier) {
    for (const auto& stockpile_element : stockpile) {
        market_information[stockpile_element.first].supply +=
            stockpile_element.second * multiplier;
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

double cqsp::common::components::Market::GetSDRatio(const entt::entity& good) {
    return market_information[good].sd_ratio;
}

double cqsp::common::components::Market::GetSupply(const entt::entity& good) {
    return market_information[good].supply;
}

double cqsp::common::components::Market::GetDemand(const entt::entity& good) {
    return market_information[good].demand;
}

double Market::GetPrice(const entt::entity& good) {
    return market_information[good].price;
}
