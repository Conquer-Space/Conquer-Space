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
#include "core/components/market.h"

namespace cqsp::core::components {
// Returns the amount the player paid for it, along with taxes?
std::pair<double, double> Market::PurchaseFromMarket(const ResourceVector& input) {
    double cost = 0;

    double tax_cost = 0;
    for (auto& [good, amount] : input) {
        tax_cost += taxation[good] * price[good] * amount;
        cost += price[good] * amount;
    }
    consumption += input;
    return std::make_pair(cost, tax_cost);
}

std::pair<double, double> Market::PurchaseFromMarket(const ResourceConsumption& input) {
    double cost = 0;

    double tax_cost = 0;
    for (auto& [good, amount] : input) {
        tax_cost += taxation[good] * price[good] * amount;
        cost += price[good] * amount;
    }
    consumption += input;
    return std::make_pair(cost, tax_cost);
}
}  // namespace cqsp::core::components
