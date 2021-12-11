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

set(CMAKE_CXX_CLANG_TIDY "")

add_subdirectory(entt)

add_subdirectory(hjson-cpp)

add_subdirectory(fmt)

# Tell spdlog to use another fmt to avoid compile conflicts.
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

add_subdirectory(glm)

if(USE_SYSTEM_OPENAL)
  find_package(OpenAL)
else()
  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
  set(ALSOFT_UPDATE_BUILD_VERSION OFF CACHE BOOL "" FORCE)
  set(LIBTYPE "STATIC" CACHE BOOL "" FORCE)
  set(OPENAL_LIBRARY "OpenAL" CACHE INTERNAL "" FORCE)
  set(OPENAL_FOUND ON CACHE BOOL "" FORCE)
  set(OPENAL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/openal-soft/include" CACHE INTERNAL "")
  add_subdirectory(openal-soft)
  # Things only the project has
  set_target_properties(common ex-common OpenAL PROPERTIES FOLDER "Libs")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if (WIN32)
add_subdirectory(glfw)
set(GLFW3_INCLUDE_PATH glfw/include) # Because imgui doesn't like it for some reason
else()
find_package(glfw3 3.3 REQUIRED)
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
    set_target_properties(freetype PROPERTIES FOLDER "Libs")
endif()

add_subdirectory(libnoise)

# Glad
set(GLAD_API "gl=4.3" CACHE STRING "" FORCE)
set(GLAD_EXPORT ON CACHE BOOL "" FORCE)
set(GLAD_EXTENSIONS "GL_AMD_debug_output" CACHE STRING "" FORCE)
add_subdirectory(glad)
# Set parent scope because this is not root directory
set(GLAD_INCLUDE_DIRS ${GLAD_INCLUDE_DIRS} PARENT_SCOPE)

# Add imgui
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${GLAD_INCLUDE_DIRS})
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
target_link_libraries(ImGui PUBLIC ${GLFW_LIBRARIES} glad)
target_compile_definitions(ImGui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
set_target_properties(ImGui PROPERTIES FOLDER "Libs")

# Copy stb_vorbis header file
configure_file(${CMAKE_SOURCE_DIR}/lib/stb/stb_vorbis.c ${CMAKE_SOURCE_DIR}/lib/include/stb_vorbis.h COPYONLY)

set_target_properties(aob fmt hjson libnoise spdlog uninstall glad glad-generate-files PROPERTIES FOLDER "Libs")
