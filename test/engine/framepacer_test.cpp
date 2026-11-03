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
#include "engine/framepacer.h"

#include <gtest/gtest.h>

#include <limits>

using cqsp::engine::FramePacer;

TEST(FramePacerTest, TickWakesBeforeTheNextFrame) {
    FramePacer pacer;
    // The last rendered frame started at 90 ms; the next is due at 120 ms.
    pacer.BeginFrame(0.090, 0.030);
    double now = 0.095;
    const double next_tick = 0.100;
    now += pacer.WaitTime(now, next_tick);
    EXPECT_DOUBLE_EQ(now, next_tick);
    EXPECT_FALSE(pacer.FrameDue(now));
    EXPECT_DOUBLE_EQ(pacer.WaitTime(now, next_tick), 0);
    // Once the tick runs, the remaining idle time still belongs to presentation.
    EXPECT_NEAR(pacer.WaitTime(now, 0.200), 0.020, 1e-12);
}

TEST(FramePacerTest, EarlyInputWakeDoesNotMoveEitherDeadline) {
    FramePacer pacer;
    pacer.BeginFrame(0.090, 0.030);
    EXPECT_NEAR(pacer.WaitTime(0.096, 0.100), 0.004, 1e-12);
    EXPECT_NEAR(pacer.WaitTime(0.099, 0.100), 0.001, 1e-12);
    EXPECT_FALSE(pacer.FrameDue(0.099));
}

TEST(FramePacerTest, PausedScenesWaitForTheFrame) {
    FramePacer pacer;
    pacer.BeginFrame(0, 1.0 / 60);
    EXPECT_DOUBLE_EQ(pacer.WaitTime(0, std::numeric_limits<double>::infinity()), 1.0 / 60);
    EXPECT_DOUBLE_EQ(pacer.WaitTime(1.0 / 60, std::numeric_limits<double>::infinity()), 0);
}

TEST(FramePacerTest, AlreadyDueTickDoesNotSleep) {
    FramePacer pacer;
    pacer.BeginFrame(0.090, 0.030);
    EXPECT_DOUBLE_EQ(pacer.WaitTime(0.105, 0.100), 0);
    EXPECT_FALSE(pacer.FrameDue(0.105));
}

TEST(FramePacerTest, SlowWorkDoesNotQueueCatchUpFrames) {
    FramePacer pacer;
    EXPECT_TRUE(pacer.FrameDue(0));
    pacer.BeginFrame(0, 1.0 / 60);
    EXPECT_TRUE(pacer.FrameDue(0.100));
    pacer.BeginFrame(0.100, 1.0 / 60);
    EXPECT_FALSE(pacer.FrameDue(0.100));
    EXPECT_NEAR(pacer.WaitTime(0.100, 1), 1.0 / 60, 1e-12);
}

TEST(FramePacerTest, LongRunningClockKeepsSubMillisecondDeadlines) {
    FramePacer pacer;
    const double now = 1000000.0;
    pacer.BeginFrame(now, 0.020);
    const double next_tick = now + 0.001;
    EXPECT_DOUBLE_EQ(now + pacer.WaitTime(now, next_tick), next_tick);
    EXPECT_GT(pacer.WaitTime(now, next_tick), 0);
    EXPECT_FALSE(pacer.FrameDue(next_tick));
}
