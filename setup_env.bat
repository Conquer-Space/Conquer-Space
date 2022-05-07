@echo off
REM This file gets vcpkg, setups the cmake build system after that
SET has_vcpkg=NOT exist vcpkg/
if %has_vcpkg% (
    git clone https://github.com/Microsoft/vcpkg.git && call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
) else (
    cd vcpkg
    git pull
    cd ..
)

REM Build in static mode because it's cooler, also because google test doesn't support it
REM and we'd like to build the tests
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
