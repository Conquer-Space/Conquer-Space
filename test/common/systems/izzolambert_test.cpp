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

#include "common/actions/maneuver/lambert/izzo.h"
#include "common/actions/maneuver/maneuver.h"
#include "common/actions/maneuver/rendezvous.h"
#include "common/components/orbit.h"
#include "common/components/units.h"

namespace cqsps = cqsp::common::systems;
namespace cqspt = cqsp::common::components::types;
TEST(IzzoTest, DISABLED_Lambert) {
    std::random_device random_device;
    std::mt19937 gen(random_device());
    const double random_range = 50.0;

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
            glm::dvec3 q = glm::cross(r0, v0);
            int rotation = 0;
            if (q.z < 0) {
                // Then it's cw
                rotation = 1;
            }
            double tof = std::uniform_real_distribution<>(0, orbit.T() * 1. / 2.)(gen);
            glm::dvec3 r1 = cqspt::OrbitTimeToVec3(orbit, tof);

            cqsps::lambert::Izzo lp(r0, r1, tof, 1, rotation != 0, 1);
            lp.solve();
            bool has_correct_orbit = false;

            for (auto& r0t : lp.get_v1()) {
                has_correct_orbit |= glm::length(v0 - r0t) < 1e-8;
            }
            EXPECT_TRUE(has_correct_orbit);
            break;
        } while (true);
    }
}
