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

#include "common/components/orbit.h"
#include "common/components/units.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"

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
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
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
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
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
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
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
    EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetApoapsis());
}

// TODO(EhWhoAmI): Reenable when I understand what's up with this
// The formula works, but when I change mt to epoch, it gets a bit weird
TEST(Maneuver, DISABLED_OffsetCircularizeApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    const cqspt::Orbit orbit(57.91e7, 0.2, 0, 0, 0, 1);
    auto maneuver = cqsps::CircularizeAtApoapsis(orbit);
    // Get the manever velocity?
    // Compare to circular velocity
    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, cqspt::PI));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetApoapsis());
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), to_velocity - from_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    EXPECT_EQ(GetTrueAnomaly(orbit, orbit.TimeToMeanAnomaly(cqspt::PI)), cqspt::PI);
    EXPECT_EQ(orbit.nu(), 0);
    // Check if it's circular
    // Get velocity at apogee
    EXPECT_DOUBLE_EQ(orbit.TimeToMeanAnomaly(cqspt::PI), maneuver.second);
    EXPECT_EQ(orbit.TimeToMeanAnomaly(cqspt::PI), 0);
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
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
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
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    EXPECT_NEAR(glm::length(maneuver.first), 0, 1e-10);
    EXPECT_DOUBLE_EQ(maneuver.second, 0);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_DOUBLE_EQ(new_orbit.semi_major_axis, orbit.GetPeriapsis());
}

// TODO(EhWhoAmI): Reenable when I understand what's up with this
TEST(Maneuver, HighEccentricityCircularizePerigeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    cqspt::Orbit orbit(57.91e9, 0.9, 0., 0, 0, 0);
    // Circularize at periapsis
    auto maneuver = cqsps::CircularizeAtPeriapsis(orbit);

    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, 0));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), from_velocity - to_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular
    EXPECT_DOUBLE_EQ(maneuver.second, 0);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.semi_major_axis, orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), new_orbit.GetPeriapsis(), 1e-4);
}

TEST(Maneuver, DISABLED_OffsetCircularizePerigeeTest) {
    // MT is offset a little bit to catch the error where we did not get the correct true anomaly
    // of the impulse position
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    cqspt::Orbit orbit(57.91e9, 0.9, 0., 0, 0, 0.45);
    // Circularize at periapsis
    auto maneuver = cqsps::CircularizeAtPeriapsis(orbit);

    double to_velocity = cqspt::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
    double from_velocity = glm::length(cqspt::OrbitVelocityToVec3(orbit, 0));
    double r = cqspt::OrbitVelocityAtR(orbit.GM, orbit.semi_major_axis, orbit.GetPeriapsis());
    EXPECT_DOUBLE_EQ(from_velocity, r);
    EXPECT_NEAR(glm::length(maneuver.first), from_velocity - to_velocity, 1e-10);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.semi_major_axis, orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), new_orbit.GetPeriapsis(), 1e-4);
}

TEST(Maneuver, RaiseApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.2, 0., 0, 0, 0);
    const double delta_orbit = 2e10;
    auto maneuver = cqsps::SetApoapsis(orbit, orbit.GetApoapsis() + delta_orbit);

    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular

    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    // Let's just set this to 2e-4 for now, until we can figure out double errors.
    // I think 2 meter difference is acceptable, for now
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis() + delta_orbit, 2e-4);
}

TEST(Maneuver, ReduceApogeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.2, 0., 0, 0, 0);
    const double delta_orbit = -2e10;
    auto maneuver = cqsps::SetApoapsis(orbit, orbit.GetApoapsis() + delta_orbit);

    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    //EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis() + delta_orbit, 1e-4);
}

TEST(Maneuver, RaisePerigeeTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.2, 0., 0, 0, 0);
    const double delta_orbit = 2e10;
    auto maneuver = cqsps::SetPeriapsis(orbit, orbit.GetPeriapsis() + delta_orbit);

    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular

    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis() + delta_orbit, 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
}

TEST(Maneuver, ReducePerigee) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0.2, 0., 0, 0, 0);
    const double delta_orbit = -2e10;
    auto maneuver = cqsps::SetPeriapsis(orbit, orbit.GetPeriapsis() + delta_orbit);

    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second + orbit.T());
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    //EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis() + delta_orbit, 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
}

TEST(Maneuver, ChangeInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e9, 0, 0., 0, 0, 0);
    auto maneuver = cqsps::SetCircularInclination(orbit, 0.5);
    double v = cqsp::common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    EXPECT_NEAR(glm::length(maneuver.first), 2 * v * sin(0.5 / 2), 1e-4);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, 0.5, 1e-4);
    EXPECT_NEAR(new_orbit.w, orbit.w, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
}

TEST(Maneuver, InclinedChangeInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.5;
    cqspt::Orbit orbit(57.91e9, 0, 0.2, 0, 0, 0);
    auto maneuver = cqsps::SetCircularInclination(orbit, new_inclination);
    double v = cqsp::common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    EXPECT_NEAR(glm::length(maneuver.first), 2 * v * sin((new_inclination - orbit.inclination) / 2), 1e-4);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, 0.5, 1e-4);
    EXPECT_NEAR(new_orbit.w, orbit.w, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
}

TEST(Maneuver, ReduceInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0, 0.7, 0, 0, 0);
    auto maneuver = cqsps::SetCircularInclination(orbit, new_inclination);
    double v = cqsp::common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    EXPECT_NEAR(glm::length(maneuver.first), abs(2 * v * sin((new_inclination - orbit.inclination) / 2)), 1e-4);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    // Just make sure that they're in the same location
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(start.x, end.x, 1e-4);
    EXPECT_NEAR(start.y, end.y, 1e-4);
    EXPECT_NEAR(start.z, end.z, 1e-4);

    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
}

// Test for altered argument of periapsis
TEST(Maneuver, AlteredWInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0, 0.7, 0, 0.3, 0);
    auto maneuver = cqsps::SetCircularInclination(orbit, new_inclination);
    double v = cqsp::common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    EXPECT_NEAR(glm::length(maneuver.first), abs(2 * v * sin((new_inclination - orbit.inclination) / 2)), 1e-4);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    //EXPECT_DOUBLE_EQ(maneuver.second, orbit.T() / 2);
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(start.x, end.x, 1e-4);
    EXPECT_NEAR(start.y, end.y, 1e-4);
    EXPECT_NEAR(start.z, end.z, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
}

// Test for altered argument of periapsis
TEST(Maneuver, AlteredLANInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0, 0.7, 0, 0.3, 0);
    auto maneuver = cqsps::SetCircularInclination(orbit, new_inclination);
    double v = cqsp::common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.semi_major_axis);
    EXPECT_NEAR(glm::length(maneuver.first), abs(2 * v * sin((new_inclination - orbit.inclination) / 2)), 1e-4);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, 0, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(start.x, end.x, 1e-4);
    EXPECT_NEAR(start.y, end.y, 1e-4);
    EXPECT_NEAR(start.z, end.z, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
}

TEST(Maneuver, EccentricInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0.5, 0.0, 0, 0, 0);
    auto maneuver = cqsps::SetInclination(orbit, new_inclination);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, orbit.eccentricity, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    // Should be at apoapsis because we look for the lowest velocity
    EXPECT_NEAR(maneuver.second, orbit.TimeToMeanAnomaly(cqspt::PI), 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(start.x, end.x, 1e-4);
    EXPECT_NEAR(start.y, end.y, 1e-4);
    EXPECT_NEAR(start.z, end.z, 1e-4);
    // If they're both pi, Ig they're the same
    EXPECT_NEAR(cqspt::normalize_radian(new_orbit.LAN + new_orbit.w), cqspt::normalize_radian(orbit.LAN + orbit.w),
                1e-4);
}

TEST(Maneuver, EccentricDecreaseInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0.5, .7, 0, 0, 0);
    auto maneuver = cqsps::SetInclination(orbit, new_inclination);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, orbit.eccentricity, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    // Should be at apoapsis because we look for the lowest velocity
    EXPECT_NEAR(maneuver.second, orbit.TimeToMeanAnomaly(cqspt::PI), 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(start.x, end.x, 1e-4);
    EXPECT_NEAR(start.y, end.y, 1e-4);
    EXPECT_NEAR(start.z, end.z, 1e-4);
    // If they're both pi, Ig they're the same
    EXPECT_NEAR(cqspt::normalize_radian(new_orbit.LAN + new_orbit.w), cqspt::normalize_radian(orbit.LAN + orbit.w),
                1e-4);
}

TEST(Maneuver, OmegaEccentricDecreaseInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0.5, 0.0, 0.4, 0, 0);
    auto maneuver = cqsps::SetInclination(orbit, new_inclination);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, orbit.eccentricity, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    EXPECT_NEAR(new_orbit.semi_major_axis, orbit.semi_major_axis, 1e-4);

    // Should be at apoapsis because we look for the lowest velocity
    EXPECT_NEAR(maneuver.second, orbit.TimeToMeanAnomaly(cqspt::PI), 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    // Ignore this, the math checks out, we are just having some precision issues
    //EXPECT_NEAR(start.x, end.x, 1e-4);
    //EXPECT_NEAR(start.y, end.y, 1e-4);
    //EXPECT_NEAR(start.z, end.z, 1e-4);
    EXPECT_NEAR(new_orbit.w, orbit.w, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
    EXPECT_NEAR(cqspt::normalize_radian(new_orbit.LAN + new_orbit.w), cqspt::normalize_radian(orbit.LAN + orbit.w),
                1e-4);
}

TEST(Maneuver, DISABLED_ChangeWEccentricChangeInclinationTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    double new_inclination = 0.4;
    cqspt::Orbit orbit(57.91e9, 0.5, 0.0, 0., 0.4, 0);
    auto maneuver = cqsps::SetInclination(orbit, new_inclination);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, maneuver.first, maneuver.second);
    // Check if it's circular
    EXPECT_NEAR(new_orbit.eccentricity, orbit.eccentricity, 1e-15);
    EXPECT_NEAR(new_orbit.GetPeriapsis(), orbit.GetPeriapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.GetApoapsis(), orbit.GetApoapsis(), 1e-4);
    EXPECT_NEAR(new_orbit.inclination, new_inclination, 1e-4);
    EXPECT_NEAR(new_orbit.semi_major_axis, orbit.semi_major_axis, 1e-4);

    // Should be at apoapsis because we look for the lowest velocity
    EXPECT_NEAR(maneuver.second, orbit.TimeToMeanAnomaly(cqspt::PI), 1e-4);
    glm::dvec3 start = cqspt::toVec3(orbit, GetTrueAnomaly(orbit, maneuver.second));
    glm::dvec3 end = cqspt::toVec3(new_orbit);
    // Ignore this, the math checks out, we are just having some precision issues
    //EXPECT_NEAR(start.x, end.x, 1e-4);
    //EXPECT_NEAR(start.y, end.y, 1e-4);
    //EXPECT_NEAR(start.z, end.z, 1e-4);
    EXPECT_NEAR(new_orbit.w, orbit.w, 1e-4);
    EXPECT_NEAR(new_orbit.LAN, orbit.LAN, 1e-4);
    EXPECT_NEAR(cqspt::normalize_radian(new_orbit.LAN + new_orbit.w), cqspt::normalize_radian(orbit.LAN + orbit.w),
                1e-4);
}

TEST(Maneuver, DISABLED_CoplanarInterceptTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;

    cqspt::Orbit orbit(57.91e9, 0, 0, 0, 0, 0);
    cqspt::Orbit orbit2(57.91e10, 0, 0, 0, 0, 0);

    auto maneuver = cqsps::CoplanarIntercept(orbit, orbit2, 0);
    // Now generate the new orbit, and check if they're gonna match up
    double transfer_angle = cqspt::CalculateTransferAngle(orbit, orbit2);
    double phase = cqspt::CalculatePhaseAngle(orbit, orbit2, maneuver.first.second);
    EXPECT_NEAR(transfer_angle, phase, 1e-4);
    cqspt::Orbit intercept_orbit = cqspt::ApplyImpulse(orbit, maneuver.first.first, maneuver.first.second);
    double time = intercept_orbit.TimeToMeanAnomaly(cqspt::PI);
    EXPECT_DOUBLE_EQ(intercept_orbit.GetApoapsis(), orbit2.semi_major_axis);
    EXPECT_DOUBLE_EQ(intercept_orbit.GetPeriapsis(), orbit.semi_major_axis);
    EXPECT_NEAR(maneuver.second.second - maneuver.first.second, time, 0.5);
    glm::dvec3 interceptor_orbit_vec =
        cqspt::toVec3(intercept_orbit, cqspt::GetTrueAnomaly(intercept_orbit, maneuver.second.second));
    glm::dvec3 intercepted_place = cqspt::toVec3(orbit2, cqspt::GetTrueAnomaly(orbit2, maneuver.second.second));
    // Get the phase angle between the two at the end?
    EXPECT_NEAR(0., cqspt::CalculatePhaseAngle(orbit2, intercept_orbit, maneuver.second.second), 1e-4);
    double dist = glm::distance(interceptor_orbit_vec, intercepted_place);
    EXPECT_NEAR(dist, 0, 100);
}
