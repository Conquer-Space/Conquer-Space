# Copyright 2021 Conquer Space

cmake_minimum_required(VERSION 3.10)

# set the project name
project(Conquer-Space VERSION 0.0)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TESTS "Enable tests" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

add_subdirectory(lib)
add_subdirectory(src)
if(TESTS)
  add_subdirectory(test)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Conquer-Space)
