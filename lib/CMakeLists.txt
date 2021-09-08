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
add_subdirectory(entt)

add_subdirectory(hjson-cpp)

add_subdirectory(fmt)

# Tell spdlog to use another fmt to avoid compile conflicts.
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

add_subdirectory(glm)

find_library(OpenAL)
if(NOT OPENAL_FOUND)
  message("OpenAL library not found")
  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
  set(ALSOFT_UPDATE_BUILD_VERSION OFF CACHE BOOL "" FORCE)
  set(LIBTYPE "STATIC" CACHE BOOL "" FORCE)
  set(OPENAL_LIBRARY "(${CMAKE_SOURCE_DIR}/lib/openal-soft")
  set(OPENAL_INCLUDE_DIR "(${CMAKE_SOURCE_DIR}/lib/openal-soft/include")
  add_subdirectory(openal-soft)
endif()

if (TESTS)
add_subdirectory(googletest)

set_target_properties(gmock gmock_main gtest_main gtest PROPERTIES FOLDER "GoogleTest")
endif()
# Add lua
add_subdirectory(LuaJIT-cmake)

add_subdirectory(sol2)

# Set properties for lua projects
set_target_properties(buildvm liblua-shared lua-static lua liblua-static minilua PROPERTIES FOLDER "Lua")

if (NOT FREETYPE_FOUND)
    add_subdirectory(freetype)
    set_target_properties(freetype "Libs")
endif()

add_subdirectory(libnoise)

set_target_properties(aob fmt hjson libnoise spdlog uninstall PROPERTIES FOLDER "Libs")

# Add imgui
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/implot)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype)
include_directories(${CMAKE_SOURCE_DIR}/lib/freetype/include)
include_directories(${GLFW3_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/cpp/)
file (GLOB IMGUI ${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp)
set(IMGUI ${IMGUI} ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.h 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.cpp 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.h 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype/imgui_freetype.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype/imgui_freetype.h
    ${CMAKE_SOURCE_DIR}/lib/implot/implot.h
    ${CMAKE_SOURCE_DIR}/lib/implot/implot.cpp
    ${CMAKE_SOURCE_DIR}/lib/implot/implot_internal.h
    ${CMAKE_SOURCE_DIR}/lib/implot/implot_items.cpp
    ${CMAKE_SOURCE_DIR}/lib/implot/implot_demo.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/cpp/imgui_stdlib.h
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/cpp/imgui_stdlib.cpp)

add_library(ImGui ${IMGUI})
target_link_libraries(ImGui PUBLIC ${GLFW_LIBRARIES})
set_target_properties(ImGui PROPERTIES FOLDER "Libs")

# Create libnoise utils library
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
add_library(libnoise-utils ${CMAKE_SOURCE_DIR}/lib/src/noiseutils.cpp)
target_link_libraries(libnoise-utils PUBLIC libnoise)
set_target_properties(ImGui PROPERTIES FOLDER "Libs")

# Copy stb_vorbis header file
configure_file(${CMAKE_SOURCE_DIR}/lib/stb/stb_vorbis.c ${CMAKE_SOURCE_DIR}/lib/include/stb_vorbis.h COPYONLY)
