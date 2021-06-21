# Copyright 2021 Conquer Space

include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${CMAKE_SOURCE_DIR}/lib/freetype/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/fmt/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/lib/win64/boost")
find_package(Boost 1.65.0 REQUIRED)
link_directories("${CMAKE_SOURCE_DIR}/lib/win64/boost/stage/lib")

file (GLOB_RECURSE CPP_FILES *.cpp)
list(FILTER CPP_FILES EXCLUDE REGEX ".*main.cpp$")
file (GLOB_RECURSE H_FILES *.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${LIB_FILES})

add_library(cqsp-core ${SOURCE_FILES})

target_link_libraries(cqsp-core PUBLIC 
    EnTT::EnTT
    hjson
    spdlog
    liblua-static
    fmt
    libnoise
    libnoise-utils
)
