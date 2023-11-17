#!/bin/sh
git -C vcpkg fetch --unshallow
git -C vcpkg describe --tags
git -C vcpkg rev-parse --short HEAD

./vcpkg/bootstrap-vcpkg.sh
cmake -B build "$@"
