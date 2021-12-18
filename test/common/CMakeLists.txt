# Conquer Space
# Copyright (C) 2021 Conquer Space

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)

include_directories(${CMAKE_SOURCE_DIR}/lib/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

add_executable(cqsp-tests ${CPP_FILES} ${H_FILES})

# Test libraries
target_link_libraries(cqsp-tests GTest::gtest GTest::gtest_main)
target_link_libraries(cqsp-tests GTest::gmock)
target_link_libraries(cqsp-tests cqsp-core)

# External libraries
target_link_libraries(cqsp-tests
    EnTT::EnTT
    hjson
    liblua-static
    spdlog::spdlog
    fmt::fmt
    libnoise
    )

set_property(TARGET cqsp-tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")

add_test(NAME cqsp-test COMMAND cqsp-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries/bin)

# Disable logging
target_compile_definitions(cqsp-tests PRIVATE SPDLOG_ACTIVE_LEVEL=1000)
