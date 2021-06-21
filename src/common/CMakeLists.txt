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

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${LIB_FILES})

foreach(_source IN ITEMS ${SOURCE_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

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
