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
#include "common/util/text.h"

#include <vector>

namespace cqsp::common::util {
int LevenshteinDistance(std::string_view string1, std::string_view string2) {
    std::vector<std::vector<int>> matrix;
    for (int i = 0; i < string1.size(); i++) {
        matrix.push_back(std::vector<int>(string2.size(), 0));
    }
    for (int i = 0; i < string1.size(); i++) {
        matrix[i][0] = i; 
    }
    return 0;
}
}