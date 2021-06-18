# Copyright 2021 Conquer Space

add_subdirectory(entt)

add_subdirectory(hjson-cpp)

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

add_subdirectory(fmt)

add_subdirectory(libnoise)

set_target_properties(aob fmt freetype hjson libnoise spdlog uninstall PROPERTIES FOLDER "Libs")