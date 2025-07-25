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

namespace cqsp::common::util {
class IRandom {
 public:
    explicit IRandom(int _seed) : seed(_seed) {}
    virtual ~IRandom() = default;
    // Random int between [a, b]
    virtual int GetRandomInt(int, int) = 0;
    // Random normal value with mean, and standard deviation
    virtual int GetRandomNormalInt(double mean, double sd) = 0;
    // Random normal value with mean, and standard deviation
    virtual double GetRandomNormal(double mean, double sd) = 0;

 protected:
    int seed;
};
}  // namespace cqsp::common::util
