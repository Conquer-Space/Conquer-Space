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

#include "common/actions/factoryconstructaction.h"
#include "common/components/area.h"
#include "common/components/market.h"
#include "common/components/resource.h"
#include "common/universe.h"

TEST(FactoryConstuctTest, ConstructTest) {
    // Create universe and recipe to generate
    cqsp::common::Universe universe;
    entt::entity city = universe.create();
    entt::entity recipe = universe.create();
    universe.emplace<cqsp::common::components::IndustrialZone>(city);
    auto& recipe_comp = universe.emplace<cqsp::common::components::Recipe>(recipe);
    recipe_comp.workers = 10;
    recipe_comp.type = cqsp::common::components::factory;
    entt::entity factory = cqsp::common::actions::CreateFactory(universe, city, recipe, 10);
    // Ensure that it has everything
    ASSERT_TRUE(universe.any_of<cqsp::common::components::Employer>(factory));
    ASSERT_EQ(universe.get<cqsp::common::components::Employer>(factory).population_needed, 10 * 10);
}

TEST(FactoryConstuctTest, ConstructExpectNoCrash) {
    cqsp::common::Universe universe;
    cqsp::common::actions::CreateFactory(universe, entt::null, entt::null, 0);
    cqsp::common::actions::CreateFactory(universe, universe.create(), entt::null, 0);
    cqsp::common::actions::CreateFactory(universe, entt::null, universe.create(), 0);
}
