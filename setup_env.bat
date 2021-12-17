@echo off
REM This file gets vcpkg, setups the cmake build system after that
if NOT exist vcpkg/ (
    git clone https://github.com/Microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
)

cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
