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

#include <fstream>
#include "common/components/coordinates.h"

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;

// Tests for input from client options
TEST(Common_OrbitTest, toVec3Test) {
    // Do the test
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 149598023;
    orb.eccentricity = 0.0167086;
    orb.inclination = 0.124878;
    orb.ascending_node = 6.08665;
    orb.argument = 1.9933;
    orb.anomaly = 6.259047404;
    orb.epoch = 1;
    orb.T = orb.CalculatePeriod();
    std::ofstream file("data.txt");
    for (int i = 0; i < 86400 * 365; i += 86400) {
        orb.epoch = i;
        glm::vec3 vec = cqspt::toVec3(orb);
        std::cout.precision(17);
        file << vec.x << " " << vec.y << " " << vec.z << std::endl;
        EXPECT_THAT(glm::length(vec), AllOf(Ge(0.98326934275),Le(1.0167257013)));
    }
    file.close();
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
