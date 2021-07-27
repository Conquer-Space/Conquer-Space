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

#include "common/components/orbit.h"

// Tests for input from client options
TEST(Common_OrbitTest, toVec2Test) {
    namespace cqspb = conquerspace::common::components::bodies;
    cqspb::Orbit orbit1;
    orbit1.semiMajorAxis = 100;
    orbit1.eccentricity = 0;
    orbit1.argument = 0;
    orbit1.theta = 0;
    cqspb::Vec2 vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360;
    cqspb::Vec2 vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.semiMajorAxis = 120;
    orbit1.eccentricity = 0.5;
    orbit1.argument = 50;
    orbit1.theta = 0;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.theta = 50;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360 + 50;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.theta = 10;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360 + 10;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);
}

TEST(Common_OrbitTest, ToRadianTest) {
    namespace cqspt = conquerspace::common::components::types;
    namespace cqspb = conquerspace::common::components::bodies;
    EXPECT_DOUBLE_EQ(cqspt::PI/2, cqspb::toRadian(90));
    EXPECT_DOUBLE_EQ(cqspt::PI, cqspb::toRadian(180));
    EXPECT_DOUBLE_EQ(cqspt::PI * 2.f, cqspb::toRadian(360));
    EXPECT_DOUBLE_EQ(cqspt::PI/6, cqspb::toRadian(30));
    EXPECT_DOUBLE_EQ(cqspt::PI/3, cqspb::toRadian(60));
    EXPECT_DOUBLE_EQ(cqspt::PI/4, cqspb::toRadian(45));
}

TEST(Common_OrbitTest, ToDegreeTest) {
    namespace cqspb = conquerspace::common::components::bodies;
    namespace cqspt = conquerspace::common::components::types;
    EXPECT_DOUBLE_EQ(30, cqspb::toDegree(cqspt::PI/6));
    EXPECT_DOUBLE_EQ(45, cqspb::toDegree(cqspt::PI/4));
    EXPECT_DOUBLE_EQ(60, cqspb::toDegree(cqspt::PI/3));
    EXPECT_DOUBLE_EQ(90, cqspb::toDegree(cqspt::PI/2));
    EXPECT_DOUBLE_EQ(180, cqspb::toDegree(cqspt::PI));
    EXPECT_DOUBLE_EQ(360, cqspb::toDegree(cqspt::PI*2));
}
