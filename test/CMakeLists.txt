# Copyright 2021 Conquer Space

enable_testing()

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/lib/win64/boost")
find_package(Boost 1.65.0 REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )
link_directories("${CMAKE_SOURCE_DIR}/lib/win64/boost/stage/lib")

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)

include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)

# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

add_executable(cqsp-tests ${CPP_FILES} ${H_FILES})

# Test libraries
target_link_libraries(cqsp-tests gtest gtest_main)
target_link_libraries(cqsp-tests cqsp-core)

# External libraries
target_link_libraries(cqsp-tests 
    glfw3_mt
    OpenGL::GL
    ImGui
    EnTT::EnTT
    hjson
    spdlog
    glm
    liblua-static
    spdlog)

add_test(NAME main-test COMMAND cqsp-tests)
