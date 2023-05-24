/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include <string>

namespace cqsp::common::util {
// Generates 6 digit hexidecimal id
inline std::string random_id() {
    int len = 6;
    static const char* alpha_num = "0123456789abcdef";
    thread_local static std::mt19937 rg {std::random_device {}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, 15);
    std::string s;

    s.reserve(len);

    while (len--) {
        s += alpha_num[pick(rg)];
    }
    return s;
}
}  // namespace cqsp::common::util
