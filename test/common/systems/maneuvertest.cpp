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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/components/orbit.h"
#include "common/components/units.h"
#include "common/systems/maneuver/maneuver.h"
TEST(Maneuver, CircularCircularizeApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0, 0, 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    // Get velocity at apogee
    EXPECT_NEAR(glm::length(maneuver.first), 0, 1e-10);
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}

TEST(Maneuver, CircularizeApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0.2, 0, 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, cqspt::PI));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    // So the expected delta v at that point should be...
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), to_velocity - from_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    // Get velocity at apogee
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}

TEST(Maneuver, InclinedCircularizeApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0.2, 0., 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, cqspt::PI));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    // So the expected delta v at that point should be...
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), to_velocity - from_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    // Get velocity at apogee
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}

TEST(Maneuver, HighEccentricityCircularizeApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.9, 0., 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, cqspt::PI));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    // So the expected delta v at that point should be...
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), to_velocity - from_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    // Get velocity at apogee
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}

TEST(Maneuver, CircularizePerigeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0.2, 0, 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtPeriapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, 0));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    // So the expected delta v at that point should be...
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), from_velocity - to_velocity, 1e-10);
    // Add one period so that we can actually calculate things
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T);
    // Check if it's circular
    EXPECT_DOUBLE_EQ(maneuver.second, 0);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.semi_major_axis, orbit.GetPeriapsis(), 1e-5);
}

TEST(Maneuver, CircularCircularizePerigeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0, 0, 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtPeriapsis(orbit);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T);
    EXPECT_NEAR(glm::length(maneuver.first), 0, 1e-10);
    EXPECT_DOUBLE_EQ(maneuver.second, 0);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetPeriapsis());
}

TEST(Maneuver, HighEccentricityCircularizePerigeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.9, 0., 0, 0, 0);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, cqspt::PI));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    // So the expected delta v at that point should be...
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), to_velocity - from_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T);
    // Check if it's circular
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}
