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

add_subdirectory(hjson-cpp)

set(SOL2_LUA_VERSION "LuaJIT" CACHE STRING "" FORCE)
add_subdirectory(sol2)

add_subdirectory(libnoise)

# Glad
set(GLAD_API "gl=4.3" CACHE STRING "" FORCE)
set(GLAD_EXPORT ON CACHE BOOL "" FORCE)
set(GLAD_EXTENSIONS "GL_AMD_debug_output, GL_EXT_framebuffer_multisample" CACHE STRING "" FORCE)

add_subdirectory(glad)


# Stb
add_library(stb STATIC stb/stb_vorbis.c)
target_include_directories(stb SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)
target_include_directories(stb SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(
    stb PROPERTIES CXX_CLANG_TIDY ""
)

set_target_properties(hjson libnoise glad glad-generate-files stb PROPERTIES FOLDER "Libs")

add_library(Tracy STATIC tracy/TracyClient.cpp)
target_compile_definitions(Tracy PRIVATE TRACY_ENABLE)
target_include_directories(Tracy PUBLIC tracy)

add_subdirectory(lunasvg)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_SVG_PLUGIN ON CACHE BOOL "" FORCE)
add_subdirectory(RmlUi)
