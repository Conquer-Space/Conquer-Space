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

#include <string>

namespace cqsp::common::components {
/// <summary>
/// Holds and calculates the tick that is on going. Date is the number of hours that has been
/// ongoing since the start of the day. Here is some rough thinking for what interval things
/// should be done at.
/// 1 tick: Combat ticks, planetary and ship movement?
/// 25 ticks: (about a day) Economic activity, though I don't think it's such a good idea to
///     put them in such a tight interval.
/// 25 * 5 = 125 ticks: (about 5 days)
/// 25 * 25 = 625 ticks: (about 26 days) Construction progress, pop growth, social changes, will
///     also be the interval that would be used for the reference value
/// 25 ^ 3 / 2 ~= 15625 ticks: (about 312 days) Momentous things and things that take a long time,
///     such as planet's climates changing
/// We will not do days and years, because these are specific to only earth, and we would like to
/// portray the different cultural differences resulting from different orbital periods and
/// rotational periods from planets, and it would be rather human-centric to assume other
/// civilizations would use our calendar. As a result, I plan to introduce a system that allows
/// the creation of different types of calendars. That would come later, though
///
/// However, this can bring some issues with statistics, because it can be hard for the player
/// to know how much they are gaining over some year, especially as we calculate over the period
/// that is irregular with a year. We can do that by calculating the ratio between the interval
/// and the calendar time. Not sure how well that would work though.
/// </summary>
class StarDate {
 public:
    // How many seconds a tick is
    static const int TIME_INCREMENT = 60;

    static const int MINUTE = 1;
    static const int HOUR = 60 * MINUTE;
    static const int DAY = 24 * HOUR;
    static const int WEEK = DAY * 7;

    static const int YEAR = DAY * 365;

    void IncrementDate() { date++; }

    uint64_t GetDate() { return date; }

    double ToSecond() { return (double)date * TIME_INCREMENT; }
    double ToDay() { return date / (float)DAY; }

    std::string ToString();
    std::string ToString(double offset);

    int GetYear();
    int GetMonth();
    int GetDay();
    int GetHour(double offset = 0.0);
    int GetMinute();

    void SetDate(unsigned int _date) { date = _date; }

    double operator()() { return ToSecond(); }

 private:
    // Time in minutes since the start date
    uint64_t date = -1;

    static const int start_date = 2000;
};
}  // namespace cqsp::common::components
