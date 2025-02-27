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

#include <random>

#include "common/components/orbit.h"
#include "common/components/units.h"
#include "common/systems/maneuver/lambert/izzo.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"

TEST(Lambert, IzzoTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is the same
    std::random_device random_device;
    std::mt19937 gen(random_device());
    const double random_range = 4.0;
    std::uniform_real_distribution<> dist(-random_range, random_range);
    glm::dvec3 r0(dist(gen), dist(gen), dist(gen));

    glm::dvec3 v0(dist(gen), dist(gen), dist(gen));
    v0 *= 0.2;
    // Now generate orbit
    cqspt::Orbit orbit = cqspt::Vec3ToOrbit(r0, v0, 1., 0);
    if (orbit.eccentricity < 1) {
        // Get orbit and then propagate it and check if it works
        glm::dvec3 start = cqspt::OrbitTimeToVec3(orbit, 0);
        // Get like a fraction of an orbit
        double tof = std::uniform_real_distribution<>(0, orbit.T() * 1. / 4)(gen);
        glm::dvec3 end = cqspt::OrbitTimeToVec3(orbit, tof);
        // Now also test the velocity
        cqsps::lambert::Izzo izzo(start, end, tof, 1, 0, 0);
        glm::dvec3 expected_velocity = cqspt::OrbitVelocityToVec3(orbit);
        glm::dvec3 start_velocity = izzo.Solve(expected_velocity);
        EXPECT_LT(glm::length(start_velocity - expected_velocity), 1e-8);
        // Generate an orbit based off that
        cqspt::Orbit generated_orbit = cqspt::Vec3ToOrbit(r0, start_velocity, 1., 0);
    } else {
        ASSERT_TRUE(false);
    }
    // Get an arbritary orbit...
}
