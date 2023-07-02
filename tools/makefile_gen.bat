@echo off
SET CC=clang
SET CXX=clang++
cmake -B build-makefile -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
