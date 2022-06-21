#!/bin/sh
./vcpkg/bootstrap-vcpkg.sh
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
