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

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/actions/shiplaunchaction.h"
#include "common/systems/movement/sysmovement.h"

namespace cqspt = cqsp::common::components::types;
class SystemsMovementTest : public ::testing::Test {
 protected:
    void SetUp() {
        cqsp::common::Universe& universe = m_game.GetUniverse();
        star_system = universe.create();

        planet = universe.create();

        auto& orbit = universe.emplace<cqspt::Orbit>(planet);
        universe.emplace<cqspt::Kinematics>(planet);
        //orbit.theta = 90;
        //orbit.semiMajorAxis = 75;

        target = universe.create();
        auto& target_orb = universe.emplace<cqspt::Orbit>(target);
        universe.emplace<cqspt::Kinematics>(target);
        //target_orb.theta = 180;
        //target_orb.semiMajorAxis = 75;
    }

    entt::entity star_system = entt::null;
    entt::entity planet = entt::null;
    entt::entity ship = entt::null;
    entt::entity target = entt::null;
    cqsp::common::Game m_game;
};

TEST_F(SystemsMovementTest, ShipMovementTest) {
    cqsp::common::Universe& universe = m_game.GetUniverse();
    {
        // Test out if the ship is created
        ship = cqsp::common::actions::CreateShip(
            universe, entt::null, cqspt::toVec3AU(universe.get<cqspt::Orbit>(planet)), star_system);
        EXPECT_TRUE(universe.valid(ship));
        bool all_of_pos_and_ship = universe.all_of<cqspt::Kinematics, cqsp::common::components::ships::Ship>(ship);
        ASSERT_TRUE(all_of_pos_and_ship);
        auto& position = universe.get<cqspt::Kinematics>(ship);
        glm::vec3 vec = cqspt::toVec3AU(universe.get<cqspt::Orbit>(planet));
        EXPECT_NEAR(position.position.x, vec.x, 4);
        EXPECT_NEAR(position.position.y, vec.y, 4);
    }
    // Test the path system
    // Due to limitations with google test, we cannot use the previous test for another test, so we
    // are putting it in the same test
    {
        // Do it a couple of times and see if it arrives
        cqsp::common::systems::SysPath system(m_game);

        // Ensure system is the same
        EXPECT_TRUE(universe.valid(ship));
        EXPECT_EQ(universe.size(), 4);

        universe.emplace_or_replace<cqspt::Kinematics>(target, cqspt::toVec3AU(universe.get<cqspt::Orbit>(target)));

        universe.emplace_or_replace<cqspt::MoveTarget>(ship, target);
        for (int i = 0; i < 1000; i++) {
            system.DoSystem();
        }
        auto& position = universe.get<cqspt::Kinematics>(ship);
        glm::vec3 vec = cqspt::toVec3AU(universe.get<cqspt::Orbit>(target));
        EXPECT_NEAR(position.position.x, vec.x, 4);
        EXPECT_NEAR(position.position.y, vec.y, 4);
    }
}
