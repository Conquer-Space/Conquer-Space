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

include(GoogleTest)

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)

# Include files
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

include_directories(${GLAD_INCLUDE_DIRS})
include_directories(${LUA_HEADERS})

add_executable(cqsp-engine-tests ${CPP_FILES} ${H_FILES})

set_property(TARGET cqsp-engine-tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")

# Test libraries
target_link_libraries(cqsp-engine-tests GTest::gtest GTest::gtest_main)
target_link_libraries(cqsp-engine-tests GTest::gmock)
target_link_libraries(cqsp-engine-tests cqsp-core cqsp-engine)
# External libraries
target_link_libraries(cqsp-engine-tests
    OpenGL::GL
    imgui::imgui
    implot::implot
    EnTT::EnTT
    hjson
    spdlog::spdlog
    glm::glm
    ${LUA_LIBRARY}
    freetype
    fmt::fmt
    libnoise
    glad
    glfw
    ${OPENAL_LIBRARY}
)

add_test(NAME cqsp-engine-test COMMAND cqsp-engine-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries/bin)

# Disable logging
target_compile_definitions(cqsp-engine-tests PRIVATE SPDLOG_ACTIVE_LEVEL=1000)
