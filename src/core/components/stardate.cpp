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
#include "core/components/stardate.h"

#include <fmt/format.h>

#include "date/date.h"

namespace cqsp::core::components {
namespace {
auto GetDateObject(int start_date, int day) {
    auto date = date::year(start_date) / 1 / 1;
    // Add hours to the date and then output
    // Add days to the date
    date = date::sys_days {date} + date::days {day};
    return date;
}
}  // namespace
std::string StarDate::ToString() const {
    // Parse the date
    auto date = GetDateObject(start_date, (int)ToDay());
    return fmt::format("{}-{}-{}", (int)date.year(), (unsigned int)date.month(), (unsigned int)date.day());
}

std::string StarDate::ToString(double offset) const {
    int day = this->date / 24;

    double diff = std::floor(this->date / 24.f) - std::floor((this->date + offset) / 24.f);
    if (diff > 0) {
        day--;
    } else if (diff < 0) {
        day++;
    }

    auto date = GetDateObject(start_date, day);
    return fmt::format("{}-{}-{}", (int)date.year(), (unsigned int)date.month(), (unsigned int)date.day());
}

int StarDate::GetYear() const {
    auto date = GetDateObject(start_date, (int)ToDay());
    return (int)date.year();
}

int StarDate::GetMonth() const {
    auto date = GetDateObject(start_date, (int)ToDay());
    return (unsigned int)date.month();
}

int StarDate::GetDay() const {
    auto date = GetDateObject(start_date, (int)ToDay());
    return (unsigned int)date.day();
}

/**
 * @param offset The hour offset
 */
int StarDate::GetHour(double offset) const { return (int)((double)date / 60 + offset) % 24; }
int StarDate::GetMinute() const { return date % 60; }
}  // namespace cqsp::core::components
