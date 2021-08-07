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

#include "common/components/movement.h"

TEST(Common_vector2test, vec_len_test) {
    namespace cqspt = conquerspace::common::components::types;
    cqspt::Vec2 vec2(3, 4);
    ASSERT_DOUBLE_EQ(vec2.length(), 5.f);
    vec2 = cqspt::Vec2(5, -12);
    ASSERT_DOUBLE_EQ(vec2.length(), 13.f);
}

TEST(Common_vector2test, vec2_normalize_test) {
    namespace cqspt = conquerspace::common::components::types;
    cqspt::Vec2 vec2(3, 4);
    cqspt::Vec2 normalized = vec2.normalize();
    ASSERT_FLOAT_EQ(normalized.length(), 1.f);
    ASSERT_FLOAT_EQ(normalized.x, 0.6f);
    ASSERT_FLOAT_EQ(normalized.y, 0.8f);
}

TEST(Common_vector2test, vec2_angle_test) {
    namespace cqspt = conquerspace::common::components::types;
    cqspt::Vec2 vec2(3, 3);
    ASSERT_DOUBLE_EQ(vec2.angle(), cqspt::PI/4);
    vec2 = cqspt::Vec2(0, 3);
    ASSERT_DOUBLE_EQ(vec2.angle(), cqspt::PI/2);
}
