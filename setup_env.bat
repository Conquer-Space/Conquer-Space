@echo off
REM This file gets vcpkg, setups the cmake build system after that
SET has_vcpkg=NOT exist vcpkg/vcpkg.exe
git -C vcpkg pull --unshallow
git -C vcpkg describe --tags
git -C vcpkg rev-parse --short HEAD

if %has_vcpkg% (
    call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
)
REM Build in static mode because it's cooler, also because google test doesn't support it
REM and we'd like to build the tests
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
