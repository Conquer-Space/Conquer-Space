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

#include "common/components/coordinates.h"

// Tests for input from client options
TEST(Common_OrbitTest, toVec2Test) {
    namespace cqspt = cqsp::common::types;
    cqspt::Orbit orbit1;
    orbit1.semiMajorAxis = 100;
    orbit1.eccentricity = 0;
    orbit1.argument = 0;
    orbit1.theta = 0;
    glm::vec3 vec1 = cqspt::toVec3(orbit1);

    orbit1.theta = 360;
    glm::vec3 vec2 = cqspt::toVec3(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.z, vec2.z);

    orbit1.semiMajorAxis = 120;
    orbit1.eccentricity = 0.5;
    orbit1.argument = 50;
    orbit1.theta = 0;
    vec1 = cqspt::toVec3(orbit1);

    orbit1.theta = 360;
    vec2 = cqspt::toVec3(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.z, vec2.z);

    orbit1.theta = 50;
    vec1 = cqspt::toVec3(orbit1);

    orbit1.theta = 360 + 50;
    vec2 = cqspt::toVec3(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.z, vec2.z);

    orbit1.theta = 10;
    vec1 = cqspt::toVec3(orbit1);

    orbit1.theta = 360 + 10;
    vec2 = cqspt::toVec3(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.z, vec2.z);
}

TEST(Common_OrbitTest, ToRadianTest) {
    namespace cqspt = cqsp::common::types;
    EXPECT_DOUBLE_EQ(cqspt::PI/2, cqspt::toRadian(90));
    EXPECT_DOUBLE_EQ(cqspt::PI, cqspt::toRadian(180));
    EXPECT_DOUBLE_EQ(cqspt::PI * 2.f, cqspt::toRadian(360));
    EXPECT_DOUBLE_EQ(cqspt::PI/6, cqspt::toRadian(30));
    EXPECT_DOUBLE_EQ(cqspt::PI/3, cqspt::toRadian(60));
    EXPECT_DOUBLE_EQ(cqspt::PI/4, cqspt::toRadian(45));
}

TEST(Common_OrbitTest, ToDegreeTest) {
    namespace cqspt = cqsp::common::types;
    EXPECT_DOUBLE_EQ(30, cqspt::toDegree(cqspt::PI/6));
    EXPECT_DOUBLE_EQ(45, cqspt::toDegree(cqspt::PI/4));
    EXPECT_DOUBLE_EQ(60, cqspt::toDegree(cqspt::PI/3));
    EXPECT_DOUBLE_EQ(90, cqspt::toDegree(cqspt::PI/2));
    EXPECT_DOUBLE_EQ(180, cqspt::toDegree(cqspt::PI));
    EXPECT_DOUBLE_EQ(360, cqspt::toDegree(cqspt::PI*2));
}
