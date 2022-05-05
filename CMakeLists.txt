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
cmake_minimum_required(VERSION 3.13)

# set the project name
project(Conquer-Space VERSION 0.0.0)
SET(CQSP_VERSION ${CMAKE_PROJECT_VERSION})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TESTS "Enable tests" ON)
set(CMAKE_CXX_CLANG_TIDY "")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

# Make everything static
if(TESTS)
  if(MSVC)
    set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif()
endif(TESTS)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:library")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
include_directories(src)

# Find all dependencies
if (UNIX)
find_package(glfw3 REQUIRED)
find_package(OpenAL REQUIRED)
set(OPENAL_LIBRARY openal CACHE STRING "" FORCE)
else()
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
set(OPENAL_LIBRARY OpenAL::OpenAL CACHE STRING "" FORCE)
endif()
find_package(OpenGL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(imgui_node_editor REQUIRED)

# Lua config
if (UNIX)
find_package(Lua REQUIRED)
set(LUA_HEADERS ${LUA_INCLUDE_DIR} CACHE STRING "" FORCE)
set(LUA_LIBRARY ${LUA_LIBRARIES} CACHE STRING "" FORCE)
else()
find_library(LUA_LIBRARY lua51)
message(${LUA_LIBRARY})
find_path(LUA_HEADERS_POS luajit/lua.h NO_CACHE)
set(LUA_HEADERS ${LUA_HEADERS_POS}luajit CACHE STRING "" FORCE)
message(${LUA_HEADERS})
endif()

find_package(Git)
if(GIT_FOUND)
execute_process(
  COMMAND git describe --tags --always --dirty --match "[0-9]*.*"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_INFO
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
elseif()
SET(GIT_INFO "00000")
endif()

add_subdirectory(lib)
add_subdirectory(src)
if(TESTS)
  add_subdirectory(test)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Conquer-Space)
