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
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)

include_directories(${CMAKE_SOURCE_DIR}/lib/tracy)
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${LUA_HEADERS})

file (GLOB_RECURSE CPP_FILES *.cpp)
list(FILTER CPP_FILES EXCLUDE REGEX ".*main.cpp$")
file (GLOB_RECURSE H_FILES *.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

foreach(_source IN ITEMS ${SOURCE_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

add_library(cqsp-engine ${SOURCE_FILES})

target_link_libraries(cqsp-engine PUBLIC 
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
    glad
    glfw
    sol2
    ${OPENAL_LIBRARY}
    stb
    RmlUi::RmlUi
    rmlui_debugger
    lunasvg
    imgui_node_editor
    assimp::assimp
    debug Tracy
    cqsp-core
)
