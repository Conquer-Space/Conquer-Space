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
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)
include_directories(${LUA_HEADERS})

add_executable(cqsp-tests ${CPP_FILES} ${H_FILES})

# Test libraries
target_link_libraries(cqsp-tests GTest::gtest)
target_link_libraries(cqsp-tests GTest::gmock)
target_link_libraries(cqsp-tests cqsp-core)
# Add engine and client for some loading functions
target_link_libraries(cqsp-tests cqsp-engine)
target_link_libraries(cqsp-tests cqsp-client)

set_property(TARGET cqsp-tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")

add_test(NAME cqsp-test COMMAND cqsp-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries/bin)

# Disable logging
target_compile_definitions(cqsp-tests PRIVATE TEST=1)

set_target_properties(cqsp-tests PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
