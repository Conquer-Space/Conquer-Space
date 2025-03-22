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
find_package(Threads)

# Include files
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${LUA_HEADERS})
include_directories(${CMAKE_SOURCE_DIR}/lib/implot)

# Hide the console window in visual studio projects - Release
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Add icon file for windows
if(MSVC)
SET(ICON_FILE "${CMAKE_SOURCE_DIR}/packaging/windows/app.rc")
else()
SET(ICON_FILE "")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/common/version.h @ONLY)

# Add libraries
add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(client)

target_compile_definitions(cqsp-client PUBLIC "$<$<CONFIG:DEBUG>:TRACY_ENABLE>")
target_compile_definitions(cqsp-core PUBLIC "$<$<CONFIG:DEBUG>:TRACY_ENABLE>")
target_compile_definitions(cqsp-engine PUBLIC "$<$<CONFIG:DEBUG>:TRACY_ENABLE>")
target_compile_definitions(cqsp-client PUBLIC "GLM_ENABLE_EXPERIMENTAL")
target_compile_definitions(cqsp-core PUBLIC "GLM_ENABLE_EXPERIMENTAL")
target_compile_definitions(cqsp-engine PUBLIC "GLM_ENABLE_EXPERIMENTAL")

add_executable(Conquer-Space main.cpp ${ICON_FILE})

# Set some msvc convinence things

target_link_libraries(Conquer-Space PRIVATE
    cqsp-core
    cqsp-client
    cqsp-engine
)

# Set output dirs
# First for the generic no-config case (e.g. with mingw)
# Second, for multi-config builds (e.g. msvc)
set_target_properties(Conquer-Space
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/binaries/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/binaries/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/binaries/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/binaries/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/binaries/bin"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/binaries/bin"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/binaries/bin"
)

set_property(TARGET Conquer-Space PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")
# -i enables universe loading immediately
set_target_properties(Conquer-Space PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "-i")

# Set cmake things
set_target_properties(Conquer-Space PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
set_target_properties(cqsp-core PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
set_target_properties(cqsp-client PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
set_target_properties(cqsp-engine PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
