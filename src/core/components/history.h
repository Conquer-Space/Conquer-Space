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

#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

namespace cqsp::core::components {
/// <summary>
/// Records the history of market.
/// </summary>
class MarketHistory {
 public:
    std::vector<std::vector<double>> price_history;
    std::vector<std::vector<double>> sd_ratio;
    std::vector<std::vector<double>> supply;
    std::vector<std::vector<double>> demand;
    std::vector<double> gdp;

    MarketHistory() {}
    MarketHistory(size_t good_count)
        : price_history(good_count), sd_ratio(good_count), supply(good_count), demand(good_count) {}
};

class LogMarket {};

class PopulationHistory {
 public:
    std::vector<double> sol;
    std::vector<double> population;
    std::vector<double> employment;
    std::vector<double> employment_rate;
};
}  // namespace cqsp::core::components
