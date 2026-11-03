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
