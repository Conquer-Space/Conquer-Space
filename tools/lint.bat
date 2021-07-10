@echo off
cpplint --linelength=100 --recursive --filter=-runtime/references,-runtime/threadsafe_fn --quiet --verbose=0 ../src ../test
