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

#include "common/systems/loading/loadutil.h"

using cqsp::common::components::types::UnitType;

class UnitLoadingTest : public ::testing::Test {
 protected:
    UnitLoadingTest() = default;

    double value;
    bool correct = false;
};

TEST_F(UnitLoadingTest, AngleDegreeReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("135deg", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AllLetterReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("asdoi", UnitType::Angle, &correct);
    EXPECT_FALSE(correct);
    value = cqsp::common::systems::loading::ReadUnit("asd oi", UnitType::Angle, &correct);
    EXPECT_FALSE(correct);
    value = cqsp::common::systems::loading::ReadUnit("asd oi ", UnitType::Angle, &correct);
    EXPECT_FALSE(correct);
    value = cqsp::common::systems::loading::ReadUnit(" asd oi ", UnitType::Angle, &correct);
    EXPECT_FALSE(correct);
}

TEST_F(UnitLoadingTest, AngleDegreeWithSpaceReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("135 deg", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AngleDegreeWithUnitlessReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("135", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AngleDegreeWithExtraSpaceReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("135 ", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AngleDegreeWithExtraSpaceReadTest2) {
    value = cqsp::common::systems::loading::ReadUnit(" 135 ", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AngleDegreeWithExtraSpaceReadTest3) {
    value = cqsp::common::systems::loading::ReadUnit(" 135", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toRadian(135));
}

TEST_F(UnitLoadingTest, AngleRadianWithSpaceReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("15 rad", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 15);
}

TEST_F(UnitLoadingTest, AngleRadianWithSpaceExponentReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("15e5 rad", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 15e5);
}

TEST_F(UnitLoadingTest, AngleWithGarbagetReadTest) {
    value = cqsp::common::systems::loading::ReadUnit("15e5 49", UnitType::Angle, &correct);
    EXPECT_FALSE(correct);
}

TEST_F(UnitLoadingTest, DistanceTest) {
    value = cqsp::common::systems::loading::ReadUnit("15e5km", UnitType::Distance, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 15e5);

    value = cqsp::common::systems::loading::ReadUnit("5AU", UnitType::Distance, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, cqsp::common::components::types::toKm(5));
}

TEST_F(UnitLoadingTest, MassTest) {
    value = cqsp::common::systems::loading::ReadUnit("15e5kg", UnitType::Mass, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 15e5);

    value = cqsp::common::systems::loading::ReadUnit("5t", UnitType::Mass, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 5000);

    value = cqsp::common::systems::loading::ReadUnit("5g", UnitType::Mass, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 0.005);
}

TEST_F(UnitLoadingTest, VolumeTest) {
    value = cqsp::common::systems::loading::ReadUnit("15e5m3", UnitType::Volume, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 15e5);
}

TEST_F(UnitLoadingTest, NegativeTest) {
    value = cqsp::common::systems::loading::ReadUnit("-15 rad", UnitType::Angle, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, -15);
}

TEST_F(UnitLoadingTest, NegativeTestChanges) {
    value = cqsp::common::systems::loading::ReadUnit("-45000 m", UnitType::Distance, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, -45);
}

TEST_F(UnitLoadingTest, MinuteTesting) {
    value = cqsp::common::systems::loading::ReadUnit("10 m", UnitType::Time, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 10 * 60);
}

TEST_F(UnitLoadingTest, HourTesting) {
    value = cqsp::common::systems::loading::ReadUnit("10 h", UnitType::Time, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 10 * 60 * 60);
}

TEST_F(UnitLoadingTest, DayTesting) {
    value = cqsp::common::systems::loading::ReadUnit("10 d", UnitType::Time, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 10 * 60 * 60 * 24);
}

TEST_F(UnitLoadingTest, SecondTesting) {
    value = cqsp::common::systems::loading::ReadUnit("10 s", UnitType::Time, &correct);
    EXPECT_TRUE(correct);
    EXPECT_DOUBLE_EQ(value, 10);
}
