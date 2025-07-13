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
#include "common/util/strip.h"

namespace cqsp::util {
std::string strip(const std::string& inpt) {
    if (inpt.empty()) {
        return "";
    }
    auto start_it = inpt.begin();
    auto end_it = inpt.rbegin();
    while (std::isspace(*start_it) != 0) ++start_it;
    if (start_it != inpt.end()) {
        while (std::isspace(*end_it) != 0) ++end_it;
    }
    return std::string(start_it, end_it.base());
}
}  // namespace cqsp::util
