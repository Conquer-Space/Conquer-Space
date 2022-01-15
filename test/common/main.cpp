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
#include <cstdio>
#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

/// <summary>
/// Custom main function for tests so that tests don't print out things from the
/// logger.
/// </summary>
GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from %s\n", __FILE__);
    // Disable all logging
    spdlog::set_level(spdlog::level::off);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
