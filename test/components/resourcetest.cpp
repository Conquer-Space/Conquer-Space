/*
 * Copyright 2021 Conquer Space
*/
#include <gtest/gtest.h>

#include "common/components/resource.h"

using conquerspace::common::components::ResourceLedger;

TEST(Common_ResourceLedger, ResourceLedgerComparison) {
    ResourceLedger first, second;
    // Set the stuff
    // Registry because it's demanding
    entt::registry reg;
    entt::entity good_one = reg.create();
    entt::entity good_two = reg.create();
    first[good_one] = 10;
    second[good_one] = 20;
    EXPECT_TRUE(first < second);
    EXPECT_FALSE(first > second);
    EXPECT_FALSE(first == second);

    first.clear();
    EXPECT_TRUE(first < second);
    EXPECT_FALSE(first > second);
    EXPECT_FALSE(first == second);

    second.clear();
    first[good_two] = 15;
    first[good_one] = 5;
    second[good_two] = 10;
    EXPECT_TRUE(first > second);
    EXPECT_FALSE(first < second);
    EXPECT_FALSE(first == second);

    second[good_two] = 15;
    second[good_one] = 5;
    EXPECT_FALSE(first > second);
    EXPECT_FALSE(first < second);
    EXPECT_TRUE(first == second);

    first.clear();
    second.clear();
    first[good_two] = 15;
    first[good_one] = 5;
    EXPECT_TRUE(first > second);
    EXPECT_FALSE(first < second);
    EXPECT_FALSE(first == second);

    first.clear();
    second.clear();
    first[good_two] = 15;
    first[good_one] = 5;
    second[good_two] = 15;
    second[good_one] = 10;
    EXPECT_FALSE(first > second);
    EXPECT_FALSE(first < second);
    EXPECT_FALSE(first == second);

    first.clear();

    EXPECT_FALSE(first > 0);
    EXPECT_FALSE(first < 0);
    EXPECT_TRUE(first == 0);
    EXPECT_TRUE(first >= 0);
    EXPECT_TRUE(first <= 0);
}
