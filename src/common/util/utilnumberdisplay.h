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

#include <fmt/format.h>

#include <cmath>
#include <string>

namespace cqsp {
namespace util {
inline std::string LongToHumanString(int64_t l) {
    if (abs(l) < 1000) {
        return fmt::format("{}", l);
    }
    static const std::string numbers[] = {
        "thousand",    "million",    "billion",    "trillion", "quadrillion",
        "quintillion", "sextillion", "septillion", "octillion"
    };
    int exponent = static_cast<int>(log10(abs(l)) / 3);

    // Now get the number
    double d = static_cast<double>(l) / pow(10, exponent * 3);

    // Round this to two decimal points
    const int precision = 100;
    d = round(d * precision) / precision;

    return fmt::format("{} {}", d, numbers[exponent - 1]);
}
}  // namespace util
}  // namespace cqsp
