@echo off
REM This file gets vcpkg, setups the cmake build system after that
SET has_vcpkg=NOT exist vcpkg/
if %has_vcpkg% (
    git clone https://github.com/Microsoft/vcpkg.git && call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
    echo %ERRORLEVEL%
)

echo Creating build files

cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
