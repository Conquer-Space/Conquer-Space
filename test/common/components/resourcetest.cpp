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

#include "common/components/resource.h"

using cqsp::common::components::ResourceLedger;

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
}

TEST(Common_ResourceLedger, ResourceLedgerDoubleComparison) {
    ResourceLedger first;

    entt::registry reg;
    entt::entity good_one = reg.create();
    entt::entity good_two = reg.create();

    EXPECT_FALSE(first > 0);
    EXPECT_FALSE(first < 0);
    EXPECT_TRUE(first == 0);
    EXPECT_TRUE(first >= 0);
    EXPECT_TRUE(first <= 0);

    first[good_one] = 0;
    first[good_two] = 0;

    EXPECT_FALSE(first > 0);
    EXPECT_FALSE(first < 0);
    EXPECT_TRUE(first == 0);
    EXPECT_TRUE(first >= 0);
    EXPECT_TRUE(first <= 0);

    first[good_one] = 10;
    first[good_two] = -5;

    EXPECT_FALSE(first > 0);
    EXPECT_FALSE(first < 0);
    EXPECT_FALSE(first == 0);
    EXPECT_FALSE(first >= 0);
    EXPECT_FALSE(first <= 0);
}

TEST(Common_ResourceLedger, HasAllResoourcesTest) {
    ResourceLedger first;
    ResourceLedger second;

    entt::registry reg;
    entt::entity good_one = reg.create();
    entt::entity good_two = reg.create();
    entt::entity good_three = reg.create();

    // Initialize the information
    first[good_one] = 10;
    first[good_two] = 0;

    second[good_one] = 10;
    second[good_three] = 8;

    EXPECT_FALSE(first.HasAllResources(second));
    EXPECT_FALSE(second.HasAllResources(first));

    // Initialize the information
    first[good_one] = 10;
    first[good_two] = 5;

    second[good_one] = 10;
    second[good_two] = 20;
    second[good_three] = 8;
    EXPECT_TRUE(second.HasAllResources(second));
}
