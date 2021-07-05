# Copyright 2021 Conquer Space

add_subdirectory(entt)

add_subdirectory(hjson-cpp)

add_subdirectory(fmt)

# Tell spdlog to use another fmt to avoid compile conflicts.
SET(SPDLOG_FMT_EXTERNAL ON)
add_subdirectory(spdlog)

add_subdirectory(glm)

add_subdirectory(googletest)

set_target_properties(gmock gmock_main gtest_main gtest PROPERTIES FOLDER "GoogleTest")

# Add lua
add_subdirectory(LuaJIT-cmake)

add_subdirectory(sol2)

# Set properties for lua projects
set_target_properties(buildvm liblua-shared lua-static lua liblua-static minilua PROPERTIES FOLDER "Lua")

add_subdirectory(freetype)


add_subdirectory(libnoise)

set_target_properties(aob fmt freetype hjson libnoise spdlog uninstall PROPERTIES FOLDER "Libs")

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
target_link_libraries(ImGui PUBLIC glfw3_mt)
set_target_properties(ImGui PROPERTIES FOLDER "Libs")

# Create libnoise utils library
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
add_library(libnoise-utils ${CMAKE_SOURCE_DIR}/lib/src/noiseutils.cpp)
target_link_libraries(libnoise-utils PUBLIC libnoise)
set_target_properties(ImGui PROPERTIES FOLDER "Libs")
