@echo off

cd ..
cd build
cmake -DTESTS=ON -DGLFW_ROOT_DIR=../lib/win64/glfw ..
cd ../tools
