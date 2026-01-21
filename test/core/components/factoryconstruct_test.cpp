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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/actions/factoryconstructaction.h"
#include "core/components/area.h"
#include "core/components/market.h"
#include "core/components/resource.h"
#include "core/universe.h"

TEST(FactoryConstuctTest, ConstructTest) {
    // Create universe and recipe to generate
    cqsp::core::Universe universe;
    cqsp::core::Node city(universe);
    cqsp::core::Node recipe(universe);
    city.emplace<cqsp::core::components::IndustrialZone>();
    auto& recipe_comp = recipe.emplace<cqsp::core::components::Recipe>();
    recipe_comp.workers = 10;
    recipe_comp.type = cqsp::core::components::ProductionType::factory;
    cqsp::core::Node factory = cqsp::core::actions::CreateFactory(city, recipe, 10);
    // Ensure that it has everything
    ASSERT_TRUE(factory.any_of<cqsp::core::components::Employer>());
    ASSERT_EQ(factory.get<cqsp::core::components::Employer>().population_needed, 10 * 10);
}

TEST(FactoryConstuctTest, ConstructExpectNoCrash) {
    cqsp::core::Universe universe;
    cqsp::core::Node city(universe);
    cqsp::core::Node recipe(universe);
    cqsp::core::Node null_node(universe, entt::null);
    EXPECT_TRUE(cqsp::core::actions::CreateFactory(null_node, null_node, 0) == entt::null);
    EXPECT_TRUE(cqsp::core::actions::CreateFactory(city, null_node, 0) == entt::null);
    EXPECT_TRUE(cqsp::core::actions::CreateFactory(null_node, recipe, 0) == entt::null);
}
