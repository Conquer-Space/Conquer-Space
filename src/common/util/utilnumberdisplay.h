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

#include <fmt/format.h>

#include <cmath>
#include <string>

namespace cqsp::util {
template <typename T>
inline constexpr std::string NumberToHumanString(const T l) {
    T absolute_value = l;
    // Ensure that it is not a unsigned int
    if constexpr (!(std::is_same<T, uint64_t>::value || std::is_same<T, uint32_t>::value)) {
        absolute_value = std::abs(l);
    }
    if (absolute_value < static_cast<T>(1000)) {
        return fmt::format("{}", l);
    }
    static const std::string numbers[] = {"k", "M",  "B",  "T",  "Qa", "Qn", "Sx", "Sp", "O",
                                          "N", "De", "Ud", "Dd", "Td", "Qd", "Qi", "Sd"};
    int exponent = static_cast<int>(std::log10(absolute_value) / 3);

    // Now get the number
    double d = static_cast<T>(l) / pow(10, exponent * 3);

    // Round this to two decimal points
    const int precision = 100;
    d = round(d * precision) / precision;

    return fmt::format("{} {}", d, numbers[exponent - 1]);
}
}  // namespace cqsp::util
