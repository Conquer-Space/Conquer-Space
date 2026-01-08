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

include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${LUA_HEADERS})
include_directories(${CMAKE_SOURCE_DIR}/lib/tracy/tracy/public)

file (GLOB_RECURSE CPP_FILES *.cpp)
file (GLOB_RECURSE H_FILES *.h)
file (GLOB_RECURSE HJSON_FILES ${CMAKE_SOURCE_DIR}/binaries/*.hjson)
file (GLOB_RECURSE LUA_FILES ${CMAKE_SOURCE_DIR}/binaries/*.lua)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${HJSON_FILES} ${LUA_FILES})

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
    spdlog::spdlog
    glm::glm
    ${LUA_LIBRARY}
    fmt::fmt
    sol2
    stb
    debug Tracy::TracyClient
)

if(NOT MSVC)
target_link_libraries(cqsp-core PUBLIC stdc++exp)
endif()
