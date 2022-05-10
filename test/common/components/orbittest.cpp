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
#include <gmock/gmock.h>

#include "common/components/coordinates.h"

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;

// Tests for input from client options
TEST(Common_OrbitTest, toVec2Test) {
    // Do the test
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 149598023;
    orb.eccentricity = 0.0167086;
    orb.inclination = cqspt::toRadian(1.57869);
    orb.ascending_node = cqspt::toRadian(-11.26064);
    orb.argument = cqspt::toRadian(114.20783);
    orb.anomaly = cqspt::toRadian(358.617);
    orb.T = orb.CalculatePeriod();
    for (int i = 0; i < 86400 * 365; i += 86400) {
        orb.epoch = i;
        std::cout << orb.epoch << std::endl;
        auto vec = cqspt::toVec3(orb);
        std::cout.precision(17);
        std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
        std::cout << glm::length(vec) << std::endl;
        //EXPECT_THAT(glm::length(vec), AllOf(Ge(147095000),Le(149598023)));
    }
    EXPECT_NEAR(orb.T/86400, 365.256363004, 0.01);
}

TEST(Common_OrbitTest, ToRadianTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(cqspt::PI/2, cqspt::toRadian(90));
    EXPECT_DOUBLE_EQ(cqspt::PI, cqspt::toRadian(180));
    EXPECT_DOUBLE_EQ(cqspt::PI * 2.f, cqspt::toRadian(360));
    EXPECT_DOUBLE_EQ(cqspt::PI/6, cqspt::toRadian(30));
    EXPECT_DOUBLE_EQ(cqspt::PI/3, cqspt::toRadian(60));
    EXPECT_DOUBLE_EQ(cqspt::PI/4, cqspt::toRadian(45));
}

TEST(Common_OrbitTest, ToDegreeTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(30, cqspt::toDegree(cqspt::PI/6));
    EXPECT_DOUBLE_EQ(45, cqspt::toDegree(cqspt::PI/4));
    EXPECT_DOUBLE_EQ(60, cqspt::toDegree(cqspt::PI/3));
    EXPECT_DOUBLE_EQ(90, cqspt::toDegree(cqspt::PI/2));
    EXPECT_DOUBLE_EQ(180, cqspt::toDegree(cqspt::PI));
    EXPECT_DOUBLE_EQ(360, cqspt::toDegree(cqspt::PI*2));
}
