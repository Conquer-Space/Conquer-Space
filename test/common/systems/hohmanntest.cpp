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

#include "common/actions/maneuver/hohmann.h"
#include "common/components/orbit.h"
#include "common/components/units.h"

TEST(Hohmann, BasicHohmannTest) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0, 0, 0, 0, 0);
    // Set the new altitude and then compute
    auto hohmann = cqsps::HohmannTransfer(orbit, 100e7);
    ASSERT_TRUE(hohmann.has_value());
    auto hohmann_pair = *hohmann;  // NOLINT
    // Apply the first maneuver
    orbit = cqspt::ApplyImpulse(orbit, hohmann_pair.first.first, hohmann_pair.first.second);
    // Apply the second maneuver
    orbit = cqspt::ApplyImpulse(orbit, hohmann_pair.second.first, hohmann_pair.second.second);
    // Make sure that the radius is the same
    EXPECT_NEAR(orbit.GetApoapsis(), 100e7, 1e-4);
}
