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
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${CMAKE_SOURCE_DIR}/lib/freetype/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/fmt/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/examples)

# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

include_directories(${CMAKE_SOURCE_DIR}/lib/implot)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype)
include_directories(${GLFW3_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${GLAD_INCLUDE_DIRS})

# Set source files
file (GLOB_RECURSE CPP_FILES *.cpp)
list(FILTER CPP_FILES EXCLUDE REGEX ".*main.cpp$")
file (GLOB_RECURSE H_FILES *.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${LIB_FILES})

foreach(_source IN ITEMS ${SOURCE_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

add_library(cqsp-client ${SOURCE_FILES})
target_link_libraries(cqsp-client PUBLIC
    cqsp-core
    cqsp-engine
    OpenGL::GL
    ImGui
    EnTT::EnTT
    hjson
    spdlog
    glm
    liblua-static
    freetype
    fmt
    libnoise
    glad
    ${OPENAL_LIBRARY}
)