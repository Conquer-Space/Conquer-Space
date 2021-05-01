# Copyright 2021 Conquer Space

# Libraries
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/lib/win64/boost")
find_package(Boost 1.65.0 REQUIRED)
link_directories("${CMAKE_SOURCE_DIR}/lib/win64/boost/stage/lib")

# All source files
file (GLOB_RECURSE CPP_FILES *.cpp)
list(FILTER CPP_FILES EXCLUDE REGEX ".*main.cpp$")
file (GLOB_RECURSE H_FILES *.h)
file (GLOB_RECURSE LIB_FILES ${CMAKE_SOURCE_DIR}/lib/src/*.c)

###
# Add imgui
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype)
include_directories(${GLFW3_INCLUDE_PATH})
file (GLOB IMGUI ${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp)
set(IMGUI ${IMGUI} ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.h 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.cpp 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.h 
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype/imgui_freetype.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype/imgui_freetype.h)

add_library(ImGui ${IMGUI})
target_link_libraries(ImGui PUBLIC glfw3_mt)

include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)
# Imgui
###

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${LIB_FILES})

# Other include files
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${CMAKE_SOURCE_DIR}/lib/freetype/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/fmt/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

# Hide the console window in visual studio projects - Release
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_library(cqsp-core ${SOURCE_FILES})
add_executable(Conquer-Space main.cpp)

set(CQSP_LIBRARIES glfw3_mt
    OpenGL::GL
    ImGui
    EnTT::EnTT
    hjson
    spdlog
    glm
    liblua-static
    freetype
    fmt
    )

target_link_libraries(cqsp-core PUBLIC 
    ${CQSP_LIBRARIES}
)

# Set some msvc convinence things

target_link_libraries(Conquer-Space PRIVATE
    ${CQSP_LIBRARIES}
    cqsp-core
)

# Set output dirs
# First for the generic no-config case (e.g. with mingw)
# Second, for multi-config builds (e.g. msvc)
set_target_properties(Conquer-Space
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
)
set_property(TARGET Conquer-Space PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")
