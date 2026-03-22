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
#include "core/util/color.h"

#include <sstream>

namespace cqsp::util {
std::tuple<int, int, int> HexToRgb(const std::string& str) {
    if (str.empty()) {
        return std::make_tuple(0, 0, 0);
    }
    std::stringstream ss;
    if (str.at(0) == '#') {
        // Then we substring it
        ss << std::hex << str.substr(1, std::string::npos).c_str();
    } else {
        ss << std::hex << str.c_str();
    }
    // If it starts with a # then we shouldn't parse it?

    unsigned int x;
    ss >> x;
    // Then we convert

    int r = (x & 0xFF0000) >> 16;
    int g = (x & 0x00FF00) >> 8;
    int b = (x & 0x0000FF);
    return std::make_tuple(r, g, b);
}
}  // namespace cqsp::util
