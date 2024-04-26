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
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "common/util/utilnumberdisplay.h"

using cqsp::common::util::LongToHumanString;

TEST(NumberDisplayTest, LongToHumanStringTest) {
    EXPECT_STREQ("1 k", LongToHumanString(1000).c_str());
    EXPECT_STREQ("1 M", LongToHumanString(1000000).c_str());
    EXPECT_STREQ("1 B", LongToHumanString(1000000000).c_str());
    EXPECT_STREQ("1 T", LongToHumanString(1000000000000).c_str());
    EXPECT_STREQ("1 Qa", LongToHumanString(1000000000000000).c_str());
    EXPECT_STREQ("1 Qn", LongToHumanString(1000000000000000000).c_str());
    EXPECT_STREQ("2.6 M", LongToHumanString(2600000).c_str());
    EXPECT_STREQ("2.6 B", LongToHumanString(2600000000).c_str());
    EXPECT_STREQ("420", LongToHumanString(420).c_str());
    EXPECT_STREQ("9.22 Qn", LongToHumanString(9223372036854775806).c_str());
    EXPECT_STREQ("-3.15 M", LongToHumanString(-3150000).c_str());
    EXPECT_STREQ("3.15 M", LongToHumanString(3145000).c_str());
}

