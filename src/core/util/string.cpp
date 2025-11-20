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
#include "core/util/string.h"

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

std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    std::string copy = s;
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = copy.find(delimiter)) != std::string::npos) {
        token = copy.substr(0, pos);
        tokens.push_back(token);
        copy.erase(0, pos + delimiter.length());
    }
    tokens.push_back(copy);

    return tokens;
}
}  // namespace cqsp::util
