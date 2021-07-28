@echo off

cd ..
mkdir build
cd build
cmake -DTESTS=ON ..
cd ../tools
