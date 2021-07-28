@echo off

cd ..
rmdir /S /Q build
mkdir build 
cd build
cmake -DTESTS=ON ..
cd ../tools
