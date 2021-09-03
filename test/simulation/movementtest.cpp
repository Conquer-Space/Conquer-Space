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

#include "common/systems/movement/sysmovement.h"
#include "common/systems/actions/shiplaunchaction.h"
#include "common/components/bodies.h"
#include "common/components/movement.h"
#include "common/components/ships.h"

namespace cqspt = cqsp::common::components::types;
class SystemsMovementTest : public ::testing::Test {
 protected:
    static void SetUpTestSuite() {
        star_system = universe.create();

        universe.emplace<cqsp::common::components::bodies::StarSystem>(star_system);

        planet = universe.create();

        auto& orbit = universe.emplace<cqspt::Orbit>(planet);
        universe.emplace<cqspt::Kinematics>(planet);
        orbit.theta = 90;
        orbit.semiMajorAxis = 75;

        target = universe.create();
        auto& target_orb = universe.emplace<cqspt::Orbit>(target);
        universe.emplace<cqspt::Kinematics>(target);
        target_orb.theta = 180;
        target_orb.semiMajorAxis = 75;
    }
    static entt::entity star_system;
    static entt::entity planet;
    static entt::entity ship;
    static entt::entity target;
    static cqsp::common::Universe universe;
};

cqsp::common::Universe SystemsMovementTest::universe = cqsp::common::Universe();
entt::entity SystemsMovementTest::star_system = entt::null;
entt::entity SystemsMovementTest::planet = entt::null;
entt::entity SystemsMovementTest::ship = entt::null;
entt::entity SystemsMovementTest::target = entt::null;

TEST_F(SystemsMovementTest, ShipCreationTest) {
    // Test out the things
    ship = cqsp::common::systems::actions::CreateShip(universe, entt::null,
                                                       planet, star_system);
    EXPECT_TRUE(universe.valid(ship));
    bool all_of_pos_and_ship = universe.all_of<cqspt::Kinematics, cqsp::common::components::ships::Ship>(ship);
    ASSERT_TRUE(all_of_pos_and_ship);
    auto& position = universe.get<cqspt::Kinematics>(ship);

    glm::vec3 vec = cqspt::tovec3(universe.get<cqspt::Orbit>(planet));
    EXPECT_NEAR(position.postion.x, vec.x, 4);
    EXPECT_NEAR(position.postion.y, vec.y, 4);
}

TEST_F(SystemsMovementTest, ShipMovementTest) {
    // Do it a couple of times and see if it arrives
    cqsp::common::systems::SysPath system;
    EXPECT_TRUE(universe.valid(ship));
    EXPECT_EQ(universe.size(), 4);

    universe.emplace_or_replace<cqspt::MoveTarget>(ship, target);
    for (int i = 0; i < 1000; i++) {
        system.DoSystem(universe);
    }
    auto& position = universe.get<cqspt::Kinematics>(ship);
    glm::vec3 vec = cqspt::tovec3(universe.get<cqspt::Orbit>(target));
    EXPECT_NEAR(position.postion.x, vec.x, 4);
    EXPECT_NEAR(position.postion.y, vec.y, 4);
}
