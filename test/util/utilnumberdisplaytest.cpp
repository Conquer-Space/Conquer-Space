/*
 * Copyright 2021 Conquer Space
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
}
