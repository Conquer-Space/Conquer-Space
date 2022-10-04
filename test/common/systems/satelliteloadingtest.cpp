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

#include "common/systems/loading/loadsatellites.h"
#include "common/components/units.h"

using namespace cqsp::common::systems::loading; // NOLINT
using namespace cqsp::common::components::types;  // NOLINT

TEST(Common_Loading_Satellites, GetEpochYearTest) {
    EXPECT_EQ(GetEpochYear(24), 2024);
    EXPECT_EQ(GetEpochYear(88), 1988);
    EXPECT_EQ(GetEpochYear(56), 2056);
    EXPECT_EQ(GetEpochYear(57), 1957);
}

TEST(Common_Loading_Satellites, GetOrbitTest) {
    auto orbit = GetOrbit("1 25544U 98067A   22275.23091245  .00058352  00000+0  10342-2 0  9998",
        "2 25544  51.6417 166.2459 0003022 250.0408 254.8118 15.49684437361780", 3.9860044188e5);
    orbit.CalculateVariables();

    // Orbital time should be roughly equal, which indicates the semi major axis is correct
    EXPECT_NEAR(15.49684437361780, 3600 * 24 / orbit.T, 0.001);
    EXPECT_NEAR(toRadian(51.6417), orbit.inclination, 0.0001);
    EXPECT_NEAR(toRadian(166.2459), orbit.LAN, 0.0001);
    EXPECT_DOUBLE_EQ(orbit.eccentricity, 0.0003022);
    EXPECT_NEAR(toRadian(250.0408), orbit.w, 0.0001);
    EXPECT_NEAR(toRadian(254.8118), orbit.M0, 0.0001);
    EXPECT_NEAR(22 * 31557600 + 275.23091245 * 86400, orbit.epoch, 0.0001);
}
