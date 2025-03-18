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
#include "common/systems/maneuver/lambert/lambert.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"

namespace cqsps = cqsp::common::systems;
namespace cqspt = cqsp::common::components::types;
TEST(Lambert, IzzoTest) {
    // Make a random orbit, apply an impulse, and ensure the position is the same
    std::random_device random_device;
    std::mt19937 gen(random_device());
    const double random_range = 50.0;
    std::uniform_real_distribution<> dist(-random_range, random_range);
    bool to_compute = true;
    while (to_compute) {
        glm::dvec3 r0(dist(gen), dist(gen), dist(gen));

        glm::dvec3 v0(dist(gen), dist(gen), dist(gen));
        v0 *= 0.2;
        // Now generate orbit
        cqspt::Orbit orbit = cqspt::Vec3ToOrbit(r0, v0, 1., 0);
        if (orbit.eccentricity < 1) {
            // Get orbit and then propagate it and check if it works
            glm::dvec3 start = cqspt::OrbitTimeToVec3(orbit, 0);
            glm::dvec3 velocity = cqspt::OrbitTimeToVelocityVec3(orbit, 0);
            // Now compute the values
            EXPECT_LT(glm::length(r0 - start), 1e-10);
            // Convert orbit back?
            // Get like a fraction of an orbit
            double T = orbit.T();
            double tof = std::uniform_real_distribution<>(0, orbit.T() * 1. / 8.)(gen);
            double T2 = tof / orbit.T();
            glm::dvec3 vel = cqspt::OrbitVelocityToVec3(orbit);
            std::cout << orbit.M0 << " << <\n";
            double sam = 1 / (2 / glm::length(start) - glm::length(vel) * glm::length(vel) / 1);
            std::cout << T << " " << 2 * cqspt::PI * sqrt(sam * sam * sam / 1) << "\n";
            glm::dvec3 end = cqspt::OrbitTimeToVec3(orbit, tof);
            for (int i = 0; i < 1000; i++) {
                tof = orbit.T() / 1000. * i;
                // Now also test the velocity
                cqsps::lambert::Izzo izzo(start, end, tof, 1, 1, 0);
                //std::cout << tof << "\n";
                //glm::dvec3 expected_velocity = cqspt::OrbitVelocityToVec3(orbit, orbit.TimeToTrueAnomaly(tof));
                glm::dvec3 expected_velocity2 = cqspt::OrbitVelocityToVec3(orbit);
                glm::dvec3 start_velocity = izzo.Solve(expected_velocity2);

                if (glm::length(start_velocity - expected_velocity2) < 1e-8) {
                    std::cout << "ALERT" << std::endl;
                    std::cout << "Expected velocities" << std::endl;
                    std::cout << "Initial velocity" << expected_velocity2.x << ", " << expected_velocity2.y << ", "
                              << expected_velocity2.z << "\n";
                    /*std::cout << "Terminal Velocity " << expected_velocity.x << ", " << expected_velocity.y << ", "
                              << expected_velocity.z << "\n";*/
                }
                //std::cout << glm::length(start_velocity - expected_velocity2) << std::endl;
            }
            //glm::dvec3 end = cqspt::OrbitTimeToVec3(orbit, tof);
            // Now also test the velocity
            /* cqsps::lambert::Izzo izzo(start, end, tof, 1, 1, 0);
            glm::dvec3 expected_velocity = cqspt::OrbitVelocityToVec3(orbit, orbit.TimeToTrueAnomaly(tof));
            glm::dvec3 expected_velocity2 = cqspt::OrbitVelocityToVec3(orbit);
            glm::dvec3 start_velocity = izzo.Solve(expected_velocity);
            std::cout << "Expected velocities" << std::endl;
            std::cout << "Initial velocity" << expected_velocity2.x << ", " << expected_velocity2.y << ", "
                      << expected_velocity2.z << "\n";
            std::cout << "Terminal Velocity " << expected_velocity.x << ", " << expected_velocity.y << ", "
                      << expected_velocity.z << "\n";

            cqsps::lambert::Izzo izzo2(start, end, tof, 1, 0, 0);
            std::cout << cqspt::OrbitVelocity(orbit.v, orbit.eccentricity, orbit.semi_major_axis, orbit.GM) << " "
                      << glm::length(expected_velocity2) << " " << glm::length(start_velocity) << "\n";
            std::cout << glm::length(expected_velocity) << " " << orbit.GetOrbitingRadius() << " "
                      << glm::length(expected_velocity2) << "\n";
            izzo2.Solve(expected_velocity);
            EXPECT_LT(glm::length(start_velocity - expected_velocity2), 1e-8);*/
            // Generate an orbit based off that
            //cqspt::Orbit generated_orbit = cqspt::Vec3ToOrbit(r0, start_velocity, 1., 0);
            to_compute = false;
        }
    }
    // Get an arbritary orbit...
}

TEST(Lambert2, IzzoTest) {
    std::random_device random_device;
    std::mt19937 gen(random_device());
    const double random_range = 50.0;
    std::cout << "Testing" << "\n";
    std::uniform_real_distribution<> dist(-random_range, random_range);
    for (int i = 0; i < 100; i++) {
        do {
            glm::dvec3 r0(dist(gen), dist(gen), dist(gen));

            glm::dvec3 v0(dist(gen), dist(gen), dist(gen));
            v0 *= 0.2;
            cqspt::Orbit orbit = cqspt::Vec3ToOrbit(r0, v0, 1, 0);
            if (orbit.eccentricity > 1) {
                continue;
            }
            double tof = std::uniform_real_distribution<>(0, orbit.T() * 1. / 2.)(gen);
            glm::dvec3 r1 = cqspt::OrbitTimeToVec3(orbit, tof);
            if (glm::length(r0 - cqspt::OrbitTimeToVec3(orbit, 0)) > 1e-8) {
                std::cout << "Unable to do stuff " << glm::length(r0 - cqspt::OrbitTimeToVec3(orbit, 0)) << std::endl;
            }
            std::cout << v0.x << " " << v0.y << " " << v0.z << "\n";
            kep_toolbox::lambert_problem lp(r0, r1, tof, 1);
            cqspt::Orbit orb2 = cqspt::Vec3ToOrbit(r0, v0, 1, 0);
            std::cout << orb2 << "\n";
            for (auto& r0t : lp.get_v1()) {
                std::cout << r0t.x << " " << r0t.y << " " << r0t.z << "\n";
                EXPECT_LT(glm::length(v0 - r0t), 1e-8);
                std::cout << cqspt::Vec3ToOrbit(r0, r0t, 1, 0) << "\n";
            }
            std::cout << "-----------" << "\n";
            break;
        } while (true);
    }
}