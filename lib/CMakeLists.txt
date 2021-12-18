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

add_subdirectory(hjson-cpp)

# Add lua
add_subdirectory(LuaJIT-cmake)

add_subdirectory(sol2)

# Set properties for lua projects
set_target_properties(buildvm liblua-shared lua-static lua liblua-static minilua PROPERTIES FOLDER "Lua")

add_subdirectory(libnoise)

# Glad
set(GLAD_API "gl=4.3" CACHE STRING "" FORCE)
set(GLAD_EXPORT ON CACHE BOOL "" FORCE)
set(GLAD_EXTENSIONS "GL_AMD_debug_output" CACHE STRING "" FORCE)

add_subdirectory(glad)

set_target_properties(hjson libnoise glad glad-generate-files PROPERTIES FOLDER "Libs")
