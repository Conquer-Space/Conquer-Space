# Copyright 2021 Conquer Space

enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)

include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/fmt/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

add_executable(cqsp-tests ${CPP_FILES} ${H_FILES})

# Test libraries
target_link_libraries(cqsp-tests gtest gtest_main)
target_link_libraries(cqsp-tests cqsp-core)

# External libraries
target_link_libraries(cqsp-tests 
    EnTT::EnTT
    hjson
    spdlog
    liblua-static
    spdlog
    fmt
    libnoise
    )

add_test(NAME main-test COMMAND cqsp-tests)
