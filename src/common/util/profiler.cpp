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
#include "common/util/profiler.h"

#include <spdlog/spdlog.h>

// Define the thing
std::map<std::string, int> profiler_information_map;

void set_time_block(const std::string& str, std::chrono::high_resolution_clock::time_point start,
                    std::chrono::high_resolution_clock::time_point end) {
    profiler_information_map[str] = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

std::map<std::string, int>& get_profile_information() { return profiler_information_map; }