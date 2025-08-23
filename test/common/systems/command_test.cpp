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
#include <gtest/gtest.h>

#include "common/components/orbit.h"
#include "common/actions/maneuver/commands.h"
#include "common/universe.h"

TEST(CommandTest, SOIHierachyTest) {
    cqsp::common::Universe universe;
    entt::entity current_body = universe.create();
    entt::entity source_body = current_body;
    // Now make the vector
    std::vector<entt::entity> expected_hierachy;
    for (int i = 0; i < 5; i++) {
        entt::entity parent_body = universe.create();
        auto& orbit = universe.emplace<cqsp::common::components::types::Orbit>(current_body);
        orbit.reference_body = parent_body;
        // Now make the parent
        current_body = parent_body;
        expected_hierachy.push_back(current_body);
    }
    universe.emplace<cqsp::common::components::types::Orbit>(current_body);
    // Generate a bunch of star systems

    std::vector<entt::entity> hierarchy = cqsp::common::systems::commands::GetSOIHierarchy(universe, source_body);
    ASSERT_EQ(expected_hierachy.size(), hierarchy.size());
    for (int i = 0; i < expected_hierachy.size(); i++) {
        EXPECT_EQ(expected_hierachy[i], hierarchy[i]);
    }
}

namespace {
// Takes in a child entity, and returns the root node of the soi tree
entt::entity GenerateSOITree(cqsp::common::Universe& universe, entt::entity child, int depth) {
    entt::entity current_body = child;
    for (int i = 0; i < 5; i++) {
        entt::entity parent_body = universe.create();
        auto& orbit = universe.emplace<cqsp::common::components::types::Orbit>(current_body);
        orbit.reference_body = parent_body;
        // Now make the parent
        current_body = parent_body;
    }
    universe.emplace<cqsp::common::components::types::Orbit>(current_body);
    return current_body;
}
}  // namespace

TEST(CommandTest, CommonSOITest) {
    // Generate a tree
    cqsp::common::Universe universe;
    entt::entity common_soi = universe.create();
    entt::entity root = GenerateSOITree(universe, common_soi, 5);
    // Now generate the various children
    entt::entity child_1 = universe.create();
    entt::entity child_2 = universe.create();
    entt::entity root_1 = GenerateSOITree(universe, child_1, 3);
    entt::entity root_2 = GenerateSOITree(universe, child_2, 6);
    // Now set the root to be the common soi
    universe.get<cqsp::common::components::types::Orbit>(root_1).reference_body = common_soi;
    universe.get<cqsp::common::components::types::Orbit>(root_2).reference_body = common_soi;
    // Now generate the arbritrary
    EXPECT_EQ(cqsp::common::systems::commands::GetCommonSOI(universe, child_1, child_2), common_soi);
}

TEST(CommandTest, CommonSOITestRoot) {
    // Generate a tree
    cqsp::common::Universe universe;
    entt::entity common_soi = universe.create();
    universe.emplace<cqsp::common::components::types::Orbit>(common_soi);
    // Now generate the various children
    entt::entity child_1 = universe.create();
    entt::entity child_2 = universe.create();
    entt::entity root_1 = GenerateSOITree(universe, child_1, 3);
    entt::entity root_2 = GenerateSOITree(universe, child_2, 6);
    // Now set the root to be the common soi
    universe.get<cqsp::common::components::types::Orbit>(root_1).reference_body = common_soi;
    universe.get<cqsp::common::components::types::Orbit>(root_2).reference_body = common_soi;
    // Now generate the arbritrary
    EXPECT_EQ(cqsp::common::systems::commands::GetCommonSOI(universe, child_1, child_2), common_soi);
}
