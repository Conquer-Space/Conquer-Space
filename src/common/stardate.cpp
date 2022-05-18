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
#include "common/stardate.h"

#include <fmt/format.h>

#include <chrono>

std::string cqsp::common::components::StarDate::ToString() {
    // Parse the date
    auto date = std::chrono::year(start_date) / 1 / 1;
    // Add hours to the date and then output
    // Add days to the date
    date = std::chrono::sys_days{date} + std::chrono::days{(int) ToDay()};
    return fmt::format("{}-{}-{}", (int) date.year(), (unsigned int) date.month(), (unsigned int) date.day());
}

int cqsp::common::components::StarDate::GetYear() {
    auto date = std::chrono::year(start_date) / 1 / 1;
    date = std::chrono::sys_days{date} + std::chrono::days{(int) ToDay()};
    return (int) date.year();
}

int cqsp::common::components::StarDate::GetMonth() {
    auto date = std::chrono::year(start_date) / 1 / 1;
    date = std::chrono::sys_days{date} + std::chrono::days{(int) ToDay()};
    return (unsigned int) date.month();
}

int cqsp::common::components::StarDate::GetDay() {
    auto date = std::chrono::year(start_date) / 1 / 1;
    date = std::chrono::sys_days{date} + std::chrono::days{(int) ToDay()};
    return (unsigned int) date.day();
}
