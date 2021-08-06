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
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "common/util/utilnumberdisplay.h"

TEST(NumberDisplayTest, LongToHumanStringTest) {
    EXPECT_STREQ("1 thousand", conquerspace::util::LongToHumanString(1000).c_str());
    EXPECT_STREQ("1 million", conquerspace::util::LongToHumanString(1000000).c_str());
    EXPECT_STREQ("1 billion", conquerspace::util::LongToHumanString(1000000000).c_str());
    EXPECT_STREQ("1 trillion", conquerspace::util::LongToHumanString(1000000000000).c_str());
    EXPECT_STREQ("1 quadrillion",
                                conquerspace::util::LongToHumanString(1000000000000000).c_str());
    EXPECT_STREQ("1 quintillion",
                                conquerspace::util::LongToHumanString(1000000000000000000).c_str());
    EXPECT_STREQ("2.6 million", conquerspace::util::LongToHumanString(2600000).c_str());
    EXPECT_STREQ("2.6 billion", conquerspace::util::LongToHumanString(2600000000).c_str());
    EXPECT_STREQ("420", conquerspace::util::LongToHumanString(420).c_str());
    EXPECT_STREQ("9.22 quintillion",
                                conquerspace::util::LongToHumanString(9223372036854775806).c_str());
    EXPECT_STREQ("-3.15 million", conquerspace::util::LongToHumanString(-3150000).c_str());
    EXPECT_STREQ("3.15 million", conquerspace::util::LongToHumanString(3145000).c_str());
    SPDLOG_INFO("Test log");
}
