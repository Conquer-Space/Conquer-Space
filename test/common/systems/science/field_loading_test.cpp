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

#include <hjson.h>

#include <iostream>

#include "common/systems/science/fields.h"
#include "common/components/science.h"
#include "common/components/name.h"

TEST(Science_FieldTest, FieldLoadingTest) {
    std::string val =
    R"(
    [
        {
            name: Science
            identifier: science
        }
        {
            name: Physics
            identifier: physics
            parent: [
                science
            ]
        }
        {
            name: Chemistry
            description: Testing
            identifier: chemistry
            parent: [
                science
            ]
            adjacent: [
                physics
            ]
        }
    ]
    )";

    // Load into hjson
    Hjson::Value hjson = Hjson::Unmarshal(val);
    cqsp::common::Universe universe;
    std::cout << val << std::endl;
    namespace cqspc = cqsp::common::components;
    cqsp::common::systems::science::LoadFields(universe, hjson);
    // Look for parents
    ASSERT_EQ(universe.fields.size(), hjson.size());
    ASSERT_NE(universe.fields.find("chemistry"), universe.fields.end());

    entt::entity chemistry = universe.fields["chemistry"];
    auto& field_comp = universe.get<cqspc::science::Field>(chemistry);

    ASSERT_EQ(field_comp.parents.size(), 1);

    // Ensure the name is science
    ASSERT_TRUE(universe.any_of<cqspc::Name>(field_comp.parents[0]));
    EXPECT_EQ(universe.get<cqspc::Name>(field_comp.parents[0]).name, "Science");

    // Check adjacents
    ASSERT_TRUE(universe.any_of<cqspc::Name>(field_comp.adjacent[0]));
    EXPECT_EQ(universe.get<cqspc::Name>(field_comp.adjacent[0]).name, "Physics");

    EXPECT_EQ(universe.get<cqspc::Description>(chemistry).description, "Testing");
}
