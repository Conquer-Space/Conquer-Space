# Copyright 2021 Conquer Space

# Include files
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/stb)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui_markdown)
include_directories(${CMAKE_SOURCE_DIR}/lib/freetype/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/fmt/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libnoise/include)
# Lua
include_directories(${CMAKE_SOURCE_DIR}/lib/lua/)
include_directories(${CMAKE_SOURCE_DIR}/lib/sol2/include)

include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/implot)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/misc/freetype)
include_directories(${GLFW3_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)

# Hide the console window in visual studio projects - Release
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()


# Add icon file for windowsSET(ICON_FILE "")
if(WIN32)
SET(ICON_FILE "${CMAKE_SOURCE_DIR}/binaries/data/app.rc")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/common/version.h @ONLY)

# Add libraries
add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(client)

add_executable(Conquer-Space main.cpp)

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
)
set_property(TARGET Conquer-Space PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/bin")
