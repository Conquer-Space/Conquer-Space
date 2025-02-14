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

#include <random>

#include "common/util/random/random.h"

namespace cqsp::common::util {
class StdRandom : public IRandom {
 public:
    explicit StdRandom(int _seed) : IRandom(_seed), random_gen(_seed) {}

    int GetRandomInt(int min, int max) {
        std::uniform_int_distribution<> dist(min, max);
        return dist(random_gen);
    }

    int GetRandomNormal(double mean, double sd) {
        std::normal_distribution<> norm {mean, sd};
        return static_cast<int>(round(norm(random_gen)));
    }

 private:
    std::mt19937 random_gen;
};
}  // namespace cqsp::common::util
