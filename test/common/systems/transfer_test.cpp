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
#include <glm/gtx/string_cast.hpp>

#include "common/actions/maneuver/transfers.h"
#include "common/systems/sysorbit_test.h"

TEST_F(SysOrbitTest, BodyVelocityVectorInOrbitPlane) {
    entt::entity moon = universe.planets["moon"];

    auto& moon_body = universe.get<cqsp::common::components::bodies::Body>(moon);
    auto& moon_orbit = universe.get<cqsp::common::components::types::Orbit>(moon);
    auto source_orbit = cqsp::common::components::types::Orbit(5000., 0.00001, 0.0002, 0.01, 0.1, 0, moon);
    source_orbit.GM = moon_body.GM;
    auto& moon_kinematics = universe.get<cqsp::common::components::types::Kinematics>(moon);
    entt::entity ship = cqsp::common::actions::LaunchShip(game.GetUniverse(), source_orbit);

    Tick(1);

    auto& ship_kinematics = universe.get<cqsp::common::components::types::Kinematics>(ship);
    // The two vectors should be parallel
    // Now compare it versus the values

    // Now it should be in the orbit plane
    const int tick_bound = 180;
    for (int i = 0; i < (int)(moon_orbit.T() / 60 / 180) + 1; i++) {
        glm::dvec3 velocity_vector =
            cqsp::common::systems::GetBodyVelocityVectorInOrbitPlane(source_orbit, moon_kinematics);
        glm::dvec3 velocity_cross = glm::cross(velocity_vector, moon_kinematics.velocity);
        glm::dvec3 source_normal = cqsp::common::components::types::GetOrbitNormal(source_orbit);
        EXPECT_NEAR(glm::dot(velocity_cross, source_normal), 0, 1e-4);
        // Also convert to true anomaly and then figure out

        double true_anomaly = cqsp::common::systems::GetBodyVelocityVectorInOrbitPlaneTrueAnomaly(
            source_orbit, ship_kinematics, moon_kinematics);
        glm::dvec3 position = cqsp::common::components::types::toVec3(source_orbit, true_anomaly);
        // Check if we're coplanar with the normal plane
        EXPECT_NEAR(glm::dot(source_normal, position), 0, 1e-5);

        EXPECT_NEAR(glm::dot(glm::normalize(position), glm::normalize(velocity_vector)), 1, 1e-5);
        Tick(180);
    }
}
