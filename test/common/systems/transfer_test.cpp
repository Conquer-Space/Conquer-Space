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
#include "common/actions/maneuver/transfers.h"
#include "common/systems/sysorbit_test.h"

TEST_F(SysOrbitTest, VectorTest) {
    Tick(1);
    entt::entity moon = universe.planets["moon"];

    auto source_orbit = cqsp::common::components::types::Orbit(1500., 0.00001, 0, 0.01, 0.1, 0, moon);
    auto& moon_kinematics = universe.get<cqsp::common::components::types::Kinematics>(moon);
    entt::entity ship = cqsp::common::actions::LaunchShip(game.GetUniverse(), source_orbit);
    glm::dvec3 velocity_vector =
        cqsp::common::systems::GetBodyVelocityVectorInOrbitPlane(source_orbit, moon_kinematics);
    auto& ship_kinematics = universe.get<cqsp::common::components::types::Kinematics>(ship);
    // The two vectors should be parallel
    // Now compare it versus the values
    moon_kinematics.velocity.x == velocity_vector.x
}
