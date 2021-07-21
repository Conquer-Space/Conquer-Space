@echo off
cpplint --linelength=100 --recursive --filter=-runtime/references,-runtime/threadsafe_fn,-build/c++11 --quiet --verbose=0 ../src ../test
