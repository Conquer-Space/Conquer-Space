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
#include "glm/glm.hpp"


TEST(Common_vector2test, vec_len_test) {
    namespace cqspt = cqsp::common::components::types;
    glm::vec3 vec3(3, 0, 4);
    ASSERT_DOUBLE_EQ(glm::length(vec3), 5.f);
    vec3 = glm::vec3(5, 0, -12);
    ASSERT_DOUBLE_EQ(glm::length(vec3), 13.f);
}

TEST(Common_vector2test, vec2_normalize_test) {
    namespace cqspt = cqsp::common::components::types;
    glm::vec3 vec3(3, 0 ,4);
    glm::vec3 normalized = glm::normalize(vec3);
    ASSERT_FLOAT_EQ(glm::length(normalized), 1.f);
    ASSERT_FLOAT_EQ(normalized.x, 0.6f);
    ASSERT_FLOAT_EQ(normalized.z, 0.8f);
}
